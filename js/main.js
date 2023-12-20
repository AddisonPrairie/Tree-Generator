import { Renderer } from "./render/renderer.js";
import { OrbitController } from "./render/orbit.js";

export function initApp(Module) {
    const renderer = new Renderer(document.querySelector("#canvas"));
    const orbitController = new OrbitController(renderer);

    orbitController.setPosition(.5, .5, 0.25);

    //set the constant cpp parameters - shadow map size

    const SHADOW_MAP_SIZE_X = 300;
    const SHADOW_MAP_SIZE_Y = 300;
    const SHADOW_MAP_SIZE_Z = 450;

    Module.ccall("init", 
        null,
        ["number", "number", "number"],
        [SHADOW_MAP_SIZE_X, SHADOW_MAP_SIZE_Y, SHADOW_MAP_SIZE_Z]);
    
    //set the variable cpp parameters - shadow behavior, tropism, etc.

    /*

    GOOD BONSAI LIKE:

    const SHADOW_Q_MAX = 18;
    const SHADOW_A = .5;
    const SHADOW_B = 1.25;
    const SHADOW_C = 3.;
    const TROPISM_DIR_X = 0.;
    const TROPISM_DIR_Y = 0.;
    const TROPISM_DIR_Z = 1.;
    const TROPISM_ETA = -1.;
    const TREE_ETA = 10.;
    const LIGHT_ETA = 7.;
    const ENERGY_ALPHA = 2.;
    const ENERGY_LAMBDA = .54;
    const PRUNE_RATIO = .48;
    const BRANCHING_ANGLE_FACTOR = 0.;

    const BRANCH_LENGTH = 1.;
    const SHADOW_RADIUS_FACTOR = 1;

    GOOD STURDY BUSH:

    const SHADOW_Q_MAX = 10;
    const SHADOW_A = .5;
    const SHADOW_B = 1.2;
    const SHADOW_C = 4.;
    const TROPISM_DIR_X = 0.;
    const TROPISM_DIR_Y = 0.;
    const TROPISM_DIR_Z = 1.;
    const TROPISM_ETA = -1.;
    const TREE_ETA = 10.;
    const LIGHT_ETA = 2.;
    const ENERGY_ALPHA = 2.;
    const ENERGY_LAMBDA = .5;
    const PRUNE_RATIO = .48;
    const BRANCHING_ANGLE_FACTOR = 1;

    const BRANCH_LENGTH = 1.;
    const SHADOW_RADIUS_FACTOR = 1;

    */

    const SHADOW_Q_MAX = 18;
    const SHADOW_A = .5;
    const SHADOW_B = 1.25;
    const SHADOW_C = 3.;
    const TROPISM_DIR_X = 0.;
    const TROPISM_DIR_Y = 0.;
    const TROPISM_DIR_Z = 1.;
    const TROPISM_ETA = -1.;
    const TREE_ETA = 10.;
    const LIGHT_ETA = 7.;
    const ENERGY_ALPHA = 2.;
    const ENERGY_LAMBDA = .54;
    const PRUNE_RATIO = .48;
    const BRANCHING_ANGLE_FACTOR = 0.;

    const BRANCH_LENGTH = 1.;
    const SHADOW_RADIUS_FACTOR = 1;

    Module.ccall(
        "setSettings", 
        null,
        ["number", "number", "number", "number", "number", "number", "number",
         "number", "number", "number", "number", "number", "number", "number",
         "number", "number"],
        [
            SHADOW_Q_MAX, SHADOW_A, SHADOW_B, SHADOW_C, TROPISM_DIR_X, TROPISM_DIR_Y, TROPISM_DIR_Z,
            TROPISM_ETA, TREE_ETA, LIGHT_ETA, ENERGY_ALPHA, ENERGY_LAMBDA, PRUNE_RATIO, BRANCHING_ANGLE_FACTOR,
            BRANCH_LENGTH, SHADOW_RADIUS_FACTOR
        ]
    );

    renderer.createSet("tree");
    renderer.transformSet("tree", "scale", [1. / SHADOW_MAP_SIZE_X, 1. / SHADOW_MAP_SIZE_X, 1. / SHADOW_MAP_SIZE_X]);

    const stepButton = document.querySelector("#step");

    let bPause = true;

    stepButton.onclick = () => {
        bPause = !bPause;
        if (bPause) 
        {
            stepButton.textContent = "start";
        } else {
            stepButton.textContent = "stop";
        }

    }

    stepButton.onclick();
    stepButton.onclick();

    function frame() {

        if (!bPause) {
            Module.ccall("step");
            renderer.pointSetFromModule("tree", Module, "getTreeMesh");

            //stepButton.onclick();
        }

        

        renderer.render();
    
        window.requestAnimationFrame(frame);
    }

    frame();
}

/*

const recordButton = document.querySelector("#record");

    let bRecording = false;
    recordButton.onclick = () => {
        bRecording = !bRecording;
        if (bRecording) {
            recordButton.textContent = "stop recording";
            startRecording();
        } else {
            recordButton.textContent = "start recording";
            mediaRecorder.stop();
        }
    };

    let mediaRecorder = null;
    let recordedChunks = [];

    function startRecording() {
        const stream = document.querySelector("#canvas").captureStream(60);
        mediaRecorder = new MediaRecorder(stream, {
            mimeType: "video/webm", codecs:"vp8"
        });

        mediaRecorder.ondataavailable = function(e) {
            if (e.data.size > 0) {
                recordedChunks.push(e.data);
            }
        };

        mediaRecorder.onstop = saveVideo;

        mediaRecorder.start();
    }

    function saveVideo() {
        const blob = new Blob(recordedChunks, { type: "video/webm" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        document.body.appendChild(a);
        a.style = "display: none";
        a.href = url;
        a.download = "tree.webm";
        a.click();
        window.setTimeout(() => {
            document.body.removeChild(a);
            window.URL.revokeObjectURL(url);  
        }, 100);
        
        recordedChunks = [];
    }


*/