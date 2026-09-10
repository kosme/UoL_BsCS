document.addEventListener('DOMContentLoaded', function () {

    document.getElementById("alias").addEventListener("change", (event) => {
        enableAliasButton();
    });

    document.getElementById("alias").addEventListener("keyup", (event) => {
        enableAliasButton();
    });

    document.querySelector("#alias-container button").addEventListener("click", (event) => {
        event.stopPropagation();
        event.preventDefault();
        sendAlias();
    });

}, false);

function enableAliasButton() {
    let alias = document.getElementById("alias").value;
    let old_alias = document.getElementById("alias-old").value;
    let button = document.querySelector("#alias-container button");

    if (alias != old_alias) {
        button.removeAttribute("disabled");
    } else {
        button.setAttribute("disabled", "disabled");
    }
}

function sendAlias() {
    var form = document.querySelector("#alias-container form");
    fetch("/alias", {
        method: "POST",
        body: new FormData(form),
    }).then((response) => {
        if (response.status == 200) {
            return response.text();
        } else {
            return "";
        }
    }).then((html) => {
        if (html.length > 0) {
            alert(html);
        }
    });
}
