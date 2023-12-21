import { Renderer } from "./render/renderer.js";
import { OrbitController } from "./render/orbit.js";
import { initUI } from "./ui/ui.js";

export async function initApp(Module) {
    //initialize renderer and orbit controller
    const renderer = new Renderer(document.querySelector("#canvas"));
    renderer.createSet("tree");

    const orbitController = new OrbitController(renderer);
    orbitController.setPosition(.5, .5, 0.25);

    //bind start/stop button
    const startStopButton = document.querySelector("#step");
    let bRunning = false;

    startStopButton.onclick = () => {
        bRunning = !bRunning;
        if (bRunning) {
            startStopButton.textContent = "stop";
        } else {
            startStopButton.textContent = "start";
        }
    };

    //wrapper for WASM web worker, also prevents overlapping calls to step() and getMesh()
    const WASM = {
        worker: null,
        bStepActive: false,
        bMeshNeedsUpdate: true,
        init: async () => {
            WASM.worker = new Worker("worker.js");

            const waitFor = new Promise((resolve, reject) => {
                WASM.worker.onmessage = () => {
                    resolve();

                    console.warn("wasm initialized...");

                    WASM.worker.onmessage = (e) => {
                        if (e.data.type === "MESH") {
                            WASM.bMeshNeedsUpdate = false;
                            renderer.setSet("tree", e.data.arr);
                        }
                        if (e.data.type === "FINISHED_STEP") {
                            WASM.bStepActive = false;
                            WASM.bMeshNeedsUpdate = true;
                        }
                    };
                };
            });

            WASM.worker.postMessage([{type: "INIT"}]);

            await waitFor;
        },
        initializeTree: (argArr) => {
            WASM.worker.postMessage(
                [{
                    type: "INITIALIZE_TREE",
                    args: argArr
                }]
            );
        },
        setSettings: (argArr) => {
            WASM.worker.postMessage(
                [{
                    type: "SET_SETTINGS",
                    args: argArr
                }]
            );
        },
        step: (argArr) => {
            if (!WASM.bStepActive) {
                WASM.bStepActive = true;

                WASM.worker.postMessage(
                    [{
                        type: "STEP",
                        args: argArr
                    }]
                )
            }
        },
        getMesh: (argArr) => {
            if (WASM.bMeshNeedsUpdate) {
                WASM.bMeshNeedsUpdate = false;

                WASM.worker.postMessage(
                    [{
                        type: "GET_MESH",
                        args: argArr
                    }]
                )
            }
        }
    };

    //wait for WASM to finish compiling
    await WASM.init();

    //create and bind UI functions
    const {
        setCallbacks,
        getInitArguments, 
        getSettingsArguments
    } = initUI((x, y, z) => {orbitController.setPosition(x, y, z);});

    function resetCallback() {
        const initArgs = getInitArguments();

        //update the scale of the mesh to the least of the 3 shadow map sizes
        renderer.clearSetTransform("tree");

        const minAxis = Math.min(Math.min(initArgs[1], initArgs[2]), initArgs[0]);
        renderer.transformSet("tree", "scale", [1. / minAxis, 1. / minAxis, 1. / minAxis]);

        WASM.initializeTree(initArgs);

        document.querySelector("#height").oninput();
        document.querySelector("#height").onchange();

        WASM.setSettings(getSettingsArguments());

        if (!WASM.bStepActive) {
            if (!bRunning) {
                startStopButton.onclick();
            }
        }
    }

    function updateCallback() {
        WASM.setSettings(getSettingsArguments());
    }

    setCallbacks(resetCallback, updateCallback);

    //create initial tree
    resetCallback();

    //main app loop
    function frame() {

        if (bRunning) {
            WASM.step();
            WASM.getMesh();
        }

        renderer.render();
    
        window.requestAnimationFrame(frame);
    }

    frame();
}