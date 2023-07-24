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

    document.querySelector("#sensors-container button").addEventListener("click", (event) => {
        fetch("/addsensor", {
            method: "GET"
        }).then(response => response.text()).then(html => alert(html));
    });

    for (const btn of document.getElementsByClassName("deleteButton")) {
        btn.addEventListener("click", (event) => {
            if (!confirm("Are you sure you want to delete this node?")) {
                return;
            }
            const myHeaders = new Headers({ 'Content-Type': 'application/x-www-form-urlencoded' });
            fetch("/deletenode", {
                method: "POST",
                body: "position=" + parseInt(btn.id),
                headers: myHeaders
            }).then(response => response.ok).then(code => {
                if (code) {
                    setTimeout(function () { location.reload(); }, 2500);
                } else {
                    alert("Deletion failed");
                }
            });
        });
    }

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
