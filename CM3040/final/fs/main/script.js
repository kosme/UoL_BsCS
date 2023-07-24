document.addEventListener('DOMContentLoaded', function () {
    displayAQIColors();

}, false);

function displayAQIColors() {
    // house, co2, waqi, room
    for (const element of document.getElementsByClassName("house")) {
        applyAQIColor(element);
    }
    for (const element of document.getElementsByClassName("co2")) {
        let value = parseInt(element.innerText);
        if (isNaN(value) == false) {
            if (value <= 600) {
                element.classList.add("good");
            } else if (value <= 1000) {
                element.classList.add("moderate");
            } else if (value <= 2500) {
                element.classList.add("notbad");
            } else if (value < 5000) {
                element.classList.add("bad");
            } else if (value < 10000) {
                element.classList.add("verybad");
            } else {
                element.classList.add("hazard");
            }
        } else {
            console.log(element);
        }
    }
    for (const element of document.getElementsByClassName("waqi")) {
        applyAQIColor(element);
    }
    for (const element of document.getElementsByClassName("room")) {
        applyAQIColor(element);
    }
    for (const element of document.getElementsByClassName("voc")) {
        let value = parseInt(element.innerText);
        if (isNaN(value) == false) {
            if (value == 0) {
                element.classList.add("good");
            } else if (value <= 3) {
                element.classList.add("moderate");
            } else if (value <= 25) {
                element.classList.add("notbad");
            } else {
                element.classList.add("bad");
            }
        } else {
            console.log(element);
        }
    }
}

function applyAQIColor(element) {
    let value = parseInt(element.innerText);
    if (isNaN(value) == false) {
        if (value < 51) {
            element.classList.add("good");
        } else if (value < 101) {
            element.classList.add("moderate");
        } else if (value < 151) {
            element.classList.add("notbad");
        } else if (value < 201) {
            element.classList.add("bad");
        } else if (value < 301) {
            element.classList.add("verybad");
        } else {
            element.classList.add("hazard");
        }
    } else {
        console.log(element);
    }
}
