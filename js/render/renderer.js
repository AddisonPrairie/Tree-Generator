export class Renderer {
    constructor (canvas) {
        this.canvas = canvas;
        this.gl = canvas.getContext('webgl');

        this.camera = {fov: 45, position: [5., 5., 5.], lookAt: [0., 0., 0.], near: 0.01, far: 100., matrix: glMatrix.mat4.create()};
        this.computeCameraMatrix();

        this.renderScale = 1.5;

        const renderer = this;
        new ResizeObserver(() => {
            renderer.onRenderSizeChange();
        }).observe(this.canvas);

        const vs_coloredLines = `
        uniform mat4 transform;

        attribute vec3 vpos;
        attribute vec3 vcol;

        varying vec3 fcol;

        void main(void) {
            gl_Position = transform * vec4(vpos, 1.0);
            fcol = vcol;
        }
        `;
        const fs_coloredLines = `
        precision highp float;

        varying vec3 fcol;

        void main(void) {
            gl_FragColor = vec4(fcol, 1.);
        }
        `;

        this.programs = {
            coloredLines: {program: this.createShaderProgram(vs_coloredLines, fs_coloredLines)}
        };

        this.programs.coloredLines.uniforms = {
            transform: this.gl.getUniformLocation(this.programs.coloredLines.program, 'transform'),

        };
        
        this.programs.coloredLines.attributes = {
            vpos: this.gl.getAttribLocation(this.programs.coloredLines.program, 'vpos'),
            vcol: this.gl.getAttribLocation(this.programs.coloredLines.program, 'vcol')
        };

        this.sets = {};
    }

    render() {
        const gl = this.gl;

        gl.clearColor(54. / 255., 54. / 255., 54. / 255., 1.);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        gl.viewport(0, 0, this.canvas.width, this.canvas.height);

        for (var name in this.sets) {
            const set = this.sets[name];

            if (!set.bRender) continue;
            if (set.arr == null) continue;

            gl.useProgram(this.programs.coloredLines.program);

            gl.enable(gl.DEPTH_TEST);

            const MVP = glMatrix.mat4.create();
            glMatrix.mat4.multiply(MVP, this.camera.matrix, set.transform);

            gl.uniformMatrix4fv(this.programs.coloredLines.uniforms.transform, false, MVP);
            
            const vBuffer = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, vBuffer);
            gl.bufferData(gl.ARRAY_BUFFER, set.arr, gl.STATIC_DRAW);

            gl.enableVertexAttribArray(this.programs.coloredLines.attributes.vpos);
            gl.vertexAttribPointer(this.programs.coloredLines.attributes.vpos, 3, gl.FLOAT, false, 6 * 4,     0);

            gl.enableVertexAttribArray(this.programs.coloredLines.attributes.vcol);
            gl.vertexAttribPointer(this.programs.coloredLines.attributes.vcol, 3, gl.FLOAT, false, 6 * 4, 3 * 4);

            gl.drawArrays(gl.LINES, 0, set.vertices);

            const error = gl.getError();

            if (error !== gl.NO_ERROR) {
                console.error('WebGL error:', error);
            }

            gl.deleteBuffer(vBuffer);
        }
    }

    setRenderScale(scale) {
        this.renderScale = scale;
    }

    onRenderSizeChange() {
        this.canvas.width = this.canvas.clientWidth   * this.renderScale;
        this.canvas.height = this.canvas.clientHeight * this.renderScale;

        this.computeCameraMatrix();
    }

    createSet(name) {
        if (name in this.sets) console.error("set already exists");

        this.sets[name] = {
            vertices: 0,
            arr: null,
            bRender: true,
            transform: glMatrix.mat4.create()
        };
    }

    deleteSet(name) {
        if (name in this.sets) {
            delete this.sets[name];
        } else {
            console.error("set does not exist");
        }
    }

    hideSet(name) {
        if (name in this.sets) {
            this.sets[name].bRender = false;
        } else {
            console.error("set does not exist");
        }
    }

    showSet(name) {
        if (name in this.sets) {
            this.sets[name].bRender = true;
        } else {
            console.error("set does not exist");
        }
    }

    transformSet(name, type, params) {
        if (name in this.sets) {
            switch (type) {
                case "scale":
                    glMatrix.mat4.scale(this.sets[name].transform, this.sets[name].transform, params);
                    break;
                default:
                    console.error("unknown transform type " + type);
            }
        } else {
            console.error("set does not exist");
        }
    }

    clearSetTransform(name) {
        if (name in this.sets) {
            this.sets[name].transform = glMatrix.mat4.create();
        } else {
            console.error("set does not exist");
        }
    }

    setSet(name, arr) {
        if (name in this.sets) {
            this.sets[name].vertices = arr.length / 6;
            this.sets[name].arr = arr;
        } else {
            console.error("set does not exist");
        }
    }

    setSetFromModule(name, Module, funcName) {
        const ptr = Module.ccall(funcName);

        const numEdges = Module.HEAP32[ptr / 4];

        this.setSet(name, Module.HEAPF32.slice(ptr / 4 + 1, ptr / 4 + 1 + numEdges * 12));
    }

    //does not copy data - can be used within a frame, but not necessarily
    //safe after the data pointed to by ptr is freed within the module
    pointSetFromModule(name, Module, funcName) {
        const ptr = Module.ccall(funcName);

        const numEdges = Module.HEAP32[ptr / 4];

        this.setSet(name, Module.HEAPF32.subarray(ptr / 4 + 1, ptr / 4 + 1 + numEdges * 12));
    }

    computeCameraMatrix() {
        const projectionTransform = glMatrix.mat4.create();
        glMatrix.mat4.perspective(projectionTransform, this.camera.fov, this.canvas.width / this.canvas.height, this.camera.near, this.camera.far);

        const positionTransform = glMatrix.mat4.create();
        glMatrix.mat4.lookAt(positionTransform, this.camera.position, this.camera.lookAt, [0., 0., 1.]);

        glMatrix.mat4.multiply(this.camera.matrix, projectionTransform, positionTransform);
    }

    createShaderProgram(vertexShaderSource, fragmentShaderSource) {
        const gl = this.gl;

        // Create vertex shader
        const vertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertexShader, vertexShaderSource);
        gl.compileShader(vertexShader);
      
        // Check for vertex shader compilation errors
        if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
          console.error('Vertex shader compilation error:', gl.getShaderInfoLog(vertexShader));
          return null;
        }
      
        // Create fragment shader
        const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragmentShader, fragmentShaderSource);
        gl.compileShader(fragmentShader);
      
        // Check for fragment shader compilation errors
        if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
          console.error('Fragment shader compilation error:', gl.getShaderInfoLog(fragmentShader));
          return null;
        }
      
        // Create shader program
        const shaderProgram = gl.createProgram();
        gl.attachShader(shaderProgram, vertexShader);
        gl.attachShader(shaderProgram, fragmentShader);
        gl.linkProgram(shaderProgram);
      
        return shaderProgram;
      }
}