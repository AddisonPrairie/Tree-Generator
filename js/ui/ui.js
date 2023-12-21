export function initUI(setCameraPosition) {
    const treeSettings = {
        SHADOW_MAP_X: {},
        SHADOW_MAP_Y: {},
        SHADOW_MAP_Z: {},
        SHADOW_A    : {},
        SHADOW_B    : {},
        SHADOW_Q_MAX: {},
        SEED        : {},
        
        SHADOW_RADIUS_FACTOR : {bNonUI: true, value: 1}
    };

    const simulationSettings = {
        TROPISM_ETA  : {},
        LIGHT_ETA    : {},
        TREE_ETA     : {},
        ENERGY_ALPHA : {},
        ENERGY_LAMBDA: {},
        PRUNE_RATIO  : {},
        SHADOW_C     : {},

        BRANCHING_ANGLE_FACTOR: {},

        BRANCH_LENGTH: {bNonUI: true, value: 1.},
        TROPISM_DIR_X: {bNonUI: true, value: 0.},
        TROPISM_DIR_Y: {bNonUI: true, value: 0.},
        TROPISM_DIR_Z: {bNonUI: true, value: 1.}
    };

    for (var key in treeSettings) {
        if ("bNonUI" in treeSettings[key]) {

        } else {
            treeSettings[key].element = document.querySelector(`#${key}`);
            treeSettings[key].value = parseFloat(
                treeSettings[key].element.value
            );
        }
    }

    for (var key in simulationSettings) {
        if ("bNonUI" in simulationSettings[key]) {

        } else {
            simulationSettings[key].element = document.querySelector(`#${key}`);
            simulationSettings[key].value = parseFloat(
                simulationSettings[key].element.value
            );
        }
    }

    //handle restricting user integer inputs

    const integerInputs = document.querySelectorAll(".integer");

    for (var i = 0; i < integerInputs.length; i++) {
        const elem = integerInputs[i];

        elem.addEventListener("input", (e) => {
            var value = elem.value;

            elem.value = value.replace(/[^0-9]+/, '');
        });

        const onComplete = (e) => {
            let intValue = parseInt(elem.value);

            if (isNaN(intValue)) intValue = 0;

            if (elem.step) {
                intValue -= (intValue % parseInt(elem.step));
            }

            if (elem.min && intValue < parseInt(elem.min)) intValue = parseInt(elem.min);
            if (elem.max && intValue > parseInt(elem.max)) intValue = parseInt(elem.max);

            elem.value = intValue;
        };

        elem.addEventListener("focusout", onComplete);
    }

    //handle restricting user float inputs

    const floatInputs = document.querySelectorAll(".minmax");

    for (var i = 0; i < floatInputs.length; i++) {
        const elem = floatInputs[i];

        const onComplete = (e) => {
            let floatValue = parseFloat(elem.value);

            if (isNaN(floatValue) || !isFinite(floatValue)) floatValue = 0.;

            if (floatValue < elem.min) floatValue = elem.min;
            if (floatValue > elem.max) floatValue = elem.max;

            elem.value = floatValue;
        };

        elem.addEventListener("focusout", onComplete);
    }

    //handle restricting the seed input

    const seed = treeSettings["SEED"].element;

    seed.addEventListener("input", (e) => {
        var value = seed.value;

        seed.value = value.replace(/[^0-9a-fA-F]+/, '');
    });

    seed.addEventListener("focusout", (e) => {
        var value = seed.value;

        const VALID = ["0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"];

        if (value.length == 0) {
            for (let i = 0; i < 10; i++) {
                value += VALID[Math.floor(Math.random() * VALID.length)];
            }
        }

        seed.value = value;
    });

    //handle reset tree & update settings buttons

    let callback_reset = null;
    let callback_update= null;

    function setCallbacks(reset, update) {
        callback_reset = reset;
        callback_update= update;
    }

    const  resetButton = document.querySelector("#reset");
    const updateButton = document.querySelector("#update");

    //gets the latest values from all of the inputs in one of our two groups
    function updateAllValuesInSet(obj) {
        for (var x in obj) {
            if ("bNonUI" in obj[x]) continue;

            const elem = obj[x].element;

            if (elem.classList.contains("integer")) {
                let intValue = parseInt(elem.value);

                if (isNaN(intValue)) intValue = 0;

                if (elem.step) {
                    intValue -= (intValue % parseInt(elem.step));
                }

                if (elem.min && intValue < parseInt(elem.min)) intValue = parseInt(elem.min);
                if (elem.max && intValue > parseInt(elem.max)) intValue = parseInt(elem.max);

                elem.value = intValue;
            }

            if (elem.classList.contains("minmax")) {
                let floatValue = parseFloat(elem.value);

                if (isNaN(floatValue) || !isFinite(floatValue)) floatValue = 0.;

                if (floatValue < elem.min) floatValue = elem.min;
                if (floatValue > elem.max) floatValue = elem.max;

                elem.value = floatValue;
            }

            obj[x].value = parseFloat(elem.value);
        }
    }

    resetButton.onclick = () => {
        updateAllValuesInSet(treeSettings);
        updateAllValuesInSet(simulationSettings);

        if (callback_reset) callback_reset();
    };

    updateButton.onclick = () => {
        updateAllValuesInSet(simulationSettings);

        if (callback_update) callback_update();
    };

    //initial call to make sure the values contained in js are the same as the UI
    updateAllValuesInSet(treeSettings);
    updateAllValuesInSet(simulationSettings);

    function getInitArguments() {
        return [
            treeSettings.SHADOW_MAP_X.value,
            treeSettings.SHADOW_MAP_Y.value,
            treeSettings.SHADOW_MAP_Z.value,
            parseInt(treeSettings.SEED.value, 16)
        ];
    }

    function getSettingsArguments() {
        return [
            treeSettings.SHADOW_Q_MAX.value, 
            treeSettings.SHADOW_A.value, 
            treeSettings.SHADOW_B.value, 
            simulationSettings.SHADOW_C.value, 
            simulationSettings.TROPISM_DIR_X.value, 
            simulationSettings.TROPISM_DIR_Y.value, 
            simulationSettings.TROPISM_DIR_Z.value,
            simulationSettings.TROPISM_ETA.value, 
            simulationSettings.TREE_ETA.value, 
            simulationSettings.LIGHT_ETA.value, 
            simulationSettings.ENERGY_ALPHA.value, 
            simulationSettings.ENERGY_LAMBDA.value, 
            simulationSettings.PRUNE_RATIO.value, 
            simulationSettings.BRANCHING_ANGLE_FACTOR.value,
            simulationSettings.BRANCH_LENGTH.value, 
            treeSettings.SHADOW_RADIUS_FACTOR.value
        ]
    }

    //set up the slider to manage the camera's height
    const height = document.querySelector("#height");

    function heightUpdate() {
        const minSize = Math.min(
            Math.min(
                treeSettings.SHADOW_MAP_Z.value, 
                treeSettings.SHADOW_MAP_X.value
            ), 
                treeSettings.SHADOW_MAP_Y.value);

        setCameraPosition(
            treeSettings.SHADOW_MAP_X.value * .5 / minSize,
            treeSettings.SHADOW_MAP_Y.value * .5 / minSize,
            parseFloat(height.value) * treeSettings.SHADOW_MAP_Z.value / minSize
        );
    }

    height.oninput = heightUpdate;
    height.onchange= heightUpdate;

    return {setCallbacks, getInitArguments, getSettingsArguments};
}
