export class OrbitController {
    constructor(renderer) {
        this.renderer = renderer;

        this.orbitControl = {
            enabled: true,
            dragging: false,
            lastX: 0,
            lastY: 0,
            rotationSpeed: .003,
            azimuthAngle: 0.,
            zenithAngle: 0.,
            radius: 1.,
            zoomSpeed: 0.005,
            position: [0., 0., 0.]
        };

        const orbit = this;

        this.renderer.canvas.addEventListener("mousedown", e => {
            orbit.orbitControl.dragging = true;
            orbit.orbitControl.lastX = e.clientX;
            orbit.orbitControl.lastY = e.clientY;
        });

        document.addEventListener("mouseup", e => {
            orbit.orbitControl.dragging = false;
        });

        document.addEventListener("mousemove", e => {
            if (orbit.orbitControl.dragging) {
                
                const dx = e.clientX - orbit.orbitControl.lastX;
                const dy = e.clientY - orbit.orbitControl.lastY;

                orbit.orbitControl.azimuthAngle += -dx * orbit.orbitControl.rotationSpeed;
                orbit.orbitControl.zenithAngle += dy * orbit.orbitControl.rotationSpeed;

                if (orbit.orbitControl.zenithAngle > Math.PI / 2.) {
                    orbit.orbitControl.zenithAngle = Math.PI / 2.001;
                }

                if (orbit.orbitControl.zenithAngle <-Math.PI / 2.) {
                    orbit.orbitControl.zenithAngle =-Math.PI / 2.001;
                }

                orbit.orbitControl.lastX = e.clientX;
                orbit.orbitControl.lastY = e.clientY;

                orbit.updateCamera();
            }
        });

        document.addEventListener("wheel", (e) => {
            orbit.orbitControl.radius += e.deltaY * orbit.orbitControl.zoomSpeed * Math.min(Math.pow(orbit.orbitControl.radius, 2.), 1.);
            
            if (orbit.orbitControl.radius < 0.1) {
                orbit.orbitControl.radius = 0.1;
            }

            orbit.updateCamera();
        });

        this.updateCamera();
    }

    updateCamera() {
        const camera = this.renderer.camera;
        const radius = this.orbitControl.radius;

        const x = this.orbitControl.position[0] + radius * Math.cos(this.orbitControl.azimuthAngle) * Math.cos(this.orbitControl.zenithAngle);
        const y = this.orbitControl.position[1] + radius * Math.sin(this.orbitControl.azimuthAngle) * Math.cos(this.orbitControl.zenithAngle);
        const z = this.orbitControl.position[2] + radius * Math.sin(this.orbitControl.zenithAngle);

        camera.lookAt = this.orbitControl.position;
        camera.position = [x, y, z];

        this.renderer.computeCameraMatrix();
    }

    setPosition(x, y, z) {
        this.orbitControl.position = [x, y, z];
        this.updateCamera();
    }
}