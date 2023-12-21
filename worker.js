importScripts("./wasm.js");

let module = null;

onmessage = async (e) => {

    const ret = e.data[0];

    switch (ret.type) {
        case "INIT":
            //console.warn("initializing WASM module in worker thread...");

            module = await Module();
            await module.ready;

            postMessage({type: "INITIALIZED"});
            
            break;

        case "INITIALIZE_TREE":
            //console.warn("initializing tree in worker thread...");

            //await verifyModule();

            initializeTree(ret.args);

            break;
        
        case "SET_SETTINGS":
            //console.warn("updating settings in worker thread...");

            //await verifyModule();

            setSettings(ret.args)

            break;
        
        case "STEP":
            //console.warn("calling step in worker thread...")

            //await verifyModule();

            step();

            postMessage({type: "FINISHED_STEP"});

            break;
        
        case "GET_MESH":
            //console.warn("calling get mesh in worker thread...");

            //await verifyModule();

            const res = getMesh();

            postMessage({type: "MESH", arr: res});

            break;
        
        default:
            console.error("unrecognized call type: " + ret.type)
    }
};

function initializeTree(argArr) {
    module.ccall("init", 
        null,
        ["number", "number", "number"],
        argArr);
}

function setSettings(argArr) {
    module.ccall(
        "setSettings", 
        null,
        ["number", "number", "number", "number", "number", "number", "number",
         "number", "number", "number", "number", "number", "number", "number",
         "number", "number"],
        argArr
    );
}

function step() {
    module.ccall("step");
}

function getMesh() {
    const ptr = module.ccall("getTreeMesh");
    const numEdges = module.HEAP32[ptr / 4];

    return module.HEAPF32.slice(
        ptr / 4 + 1, ptr / 4 + 1 + numEdges * 12
    );
}