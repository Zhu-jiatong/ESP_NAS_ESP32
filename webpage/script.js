function _(el) {
    return document.getElementById(el);
}

window.onload = function () {
    listFilesButton();
    fetch('/cardinfo').then(res => res.json()).then(data => {
        _("cardinfo").max = data['max'];
        _("cardinfo").value = data['val'];
        _("usedsd").innerHTML = data['usedsd'];
        _("freesd").innerHTML = data['freesd'];
    })
    _("usedsd").innerHTML = _("freesd").innerHTML = "...";
}

function listFilesButton() {
    _("details").innerHTML = "loading...";
    _('detailsheader').innerHTML = '';
    fetch('/listfiles').then(res => res.json()).then(data => {
        _("details").innerHTML = '<table id="fList"><tr><th></th><th>Name</th><th>Size</th></tr></table>';
        for (const fileKey in data) {
            let appStr = '<tr id="fileLn">' +
                '<td id="fIcon" onclick="window.open(\'' + data[fileKey]['view'] + '\')">' + fileIcon(fileKey) + '</td>' +
                '<td onclick="window.open(\'' + data[fileKey]['view'] + '\')">' + fileKey + '</td><td>' + data[fileKey]['size'] + '</td>';
            if (!data[fileKey]['isDir'])
                appStr += "<td>" + '<button id="down"' + 'onclick="downloadDeleteButton(\'' + fileKey + '\', \'download\')">&#xe896;</button>' + "</td><td>" + '<button id="del" onclick="downloadDeleteButton(\'' + fileKey + '\', \'delete\')">&#xe74d;</button>' + "</td>";
            appStr += "</tr>";
            _('fList').insertAdjacentHTML('beforeend', appStr);
        }
    })
}

function downloadDeleteButton(filename, action) {
    var urltocall = "/file?name=" + filename + "&action=" + action;
    if (action == "delete")
        fetch(urltocall).then(res => res.text()).then(data => _("status").innerHTML = data).then(listFilesButton());
    if (action == "download") {
        _("status").innerHTML = "";
        window.location.replace(urltocall);
    }
}

function showUploadButtonFancy() {
    _("detailsheader").innerHTML = "<h3>Upload File<h3>"
    _("status").innerHTML = "";
    var uploadform = "<form method = \"POST\" action = \"/\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"data\"/><input type=\"submit\" name=\"upload\" value=\"Upload\" title = \"Upload File\"></form>"
    _("details").innerHTML = uploadform;
    var uploadform =
        "<form id=\"upload_form\" enctype=\"multipart/form-data\" method=\"post\">" +
        "<input type=\"file\" name=\"file1\" id=\"file1\" multiple onchange=\"uploadFile()\"><br>" +
        "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:100%;\"></progress>" +
        "<h3 id=\"status\"></h3>" +
        "<p id=\"loaded_n_total\"></p>" +
        "</form>";
    _("details").innerHTML = uploadform;
}
function uploadFile() {
    var formdata = new FormData();
    Array.from(_("file1").files).forEach(file => {
        formdata.append("files", file);
    });
    var ajax = new XMLHttpRequest();
    ajax.upload.addEventListener("progress", progressHandler, false);
    ajax.addEventListener("load", completeHandler, false); // doesnt appear to ever get called even upon success
    ajax.addEventListener("error", errorHandler, false);
    ajax.addEventListener("abort", abortHandler, false);
    ajax.open("POST", "/");
    ajax.send(formdata);
}
function progressHandler(event) {
    _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
    var percent = (event.loaded / event.total) * 100;
    _("progressBar").value = Math.round(percent);
    _("status").innerHTML = Math.round(percent) + "% uploaded... please wait";
    if (percent >= 100) {
        _("status").innerHTML = "Please wait, writing file to filesystem";
    }
}
function completeHandler(event) {
    _("status").innerHTML = "Upload Complete";
    _("progressBar").value = 0;
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", "/listfiles", false);
    xmlhttp.send();
    _("status").innerHTML = "File Uploaded";
    listFilesButton();
}
function errorHandler(event) {
    _("status").innerHTML = "Upload Failed";
}
function abortHandler(event) {
    _("status").innerHTML = "inUpload Aborted";
}
function fileIcon(filename) {
    switch (filename.split('.').pop()) {
        case 'jpg':
        case 'jpeg':
        case 'png':
        case 'gif':
        case 'ico':
            return '&#xe8b9;';
        case 'mp4':
        case 'mov':
            return '&#xe714;';
        case 'mp3':
            return '&#xe8d6;';
        case 'js':
        case 'css':
        case 'html':
        case 'bin':
            return '&#xe943;';
        case 'pdf':
        case 'docx':
        case 'doc':
            return '&#xe8a5;';
        case 'ttf':
            return '&#xe8d2;'

        default:
            return '&#xe8b7;'
    }
}