document.addEventListener('DOMContentLoaded', function () {
    // Enable toggling password visibility
    document.getElementById("allow-visible-btn").addEventListener("click", (event) => {
        event.stopPropagation();
        togglePasswordVisibility();
    });

    // Refresh list of detected networks
    document.getElementById("refresh-net-btn").addEventListener("click", (event) => {
        event.stopPropagation();
        refreshNetworks();
    });

    // Send Form
    document.getElementById("submit-form").addEventListener("click", (event) => {
        event.stopPropagation();
        sendForm();
    });

    // Avoid sending form when Enter is key pressed
    document.getElementById("password-hidden").addEventListener("keypress", (event) => {
        if (event.key == "Enter") {
            event.preventDefault();
            sendForm();
        }
    });
}, false);

function togglePasswordVisibility() {
    var passwordHidden = document.getElementById("password-hidden");
    var button = document.getElementById("allow-visible-btn");
    var isVisible = button.classList.contains("active");

    if (isVisible) {
        passwordHidden.attributes.type.value = "password";
        button.classList.replace("active", "inactive");
    } else {
        passwordHidden.attributes.type.value = "text";
        button.classList.replace("inactive", "active");
    }
}

function refreshNetworks() {
    var networkSelector = document.getElementById("network-select");
    const oReq = new XMLHttpRequest();
    oReq.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            let response = JSON.parse(oReq.response);
            // Remove all children
            networkSelector.innerHTML = "";
            new Set(response["names"]).forEach(element => {
                let node = new Option(element, element);
                networkSelector.appendChild(node);
            });
        }
    }
    oReq.open("GET", "/networks");
    oReq.send();
}

function sendForm() {
    var form = document.getElementById("network-form");
    fetch("/", {
        method: "POST",
        body: new FormData(form),
    }).then(response => response.text()).then(html => alert(html));

    // Clear password field
    document.getElementById("password-hidden").value = "";
}
