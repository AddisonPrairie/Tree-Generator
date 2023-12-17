import { Renderer } from "./render/renderer.js";
import { OrbitController } from "./render/orbit.js";

export function initApp(Module) {
    const renderer = new Renderer(document.querySelector("#canvas"));
    const orbitController = new OrbitController(renderer);

    orbitController.setPosition(.5, .5, 0.);

    Module.ccall("init");

    renderer.createSet("tree");
    renderer.transformSet("tree", "scale", [1. / 400., 1. / 400., 1. / 400.]);

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
        const stream = document.querySelector("#canvas").captureStream(30);
        mediaRecorder = new MediaRecorder(stream, { mimeType: "video/webm" });

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

    function frame() {

        if (!bPause) {
            Module.ccall("step");
        }

        renderer.pointSetFromModule("tree", Module, "getTreeMesh");

        renderer.render();
    
        window.requestAnimationFrame(frame);
    }

    frame();
}