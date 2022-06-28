function _(el) {
    return document.getElementById(el);
}

window.onload = listFilesButton();

function listFilesButton() {
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", "/listfiles", false);
    xmlhttp.send();
    _("detailsheader").innerHTML = "<h3>Files<h3>";
    _("details").innerHTML = xmlhttp.responseText;
}
function downloadDeleteButton(filename, action) {
    var urltocall = "/file?name=" + filename + "&action=" + action;
    xmlhttp = new XMLHttpRequest();
    if (action == "delete") {
        xmlhttp.open("GET", urltocall, false);
        xmlhttp.send();
        _("status").innerHTML = xmlhttp.responseText;
        xmlhttp.open("GET", "/listfiles", false);
        xmlhttp.send();
        _("details").innerHTML = xmlhttp.responseText;
    }
    if (action == "download") {
        _("status").innerHTML = "";
        window.open(urltocall, "_blank");
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
    _("detailsheader").innerHTML = "<h3>Files<h3>";
    _("details").innerHTML = xmlhttp.responseText;
}
function errorHandler(event) {
    _("status").innerHTML = "Upload Failed";
}
function abortHandler(event) {
    _("status").innerHTML = "inUpload Aborted";
}
function STACfg() {
    var urltocall = "/postSTA?sta_ssid=" + document.sta.sta_ssid.value + "&sta_psk=" + document.sta.sta_psk.value;
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();
    _("STAstatus").innerHTML = xmlhttp.responseText;
    xmlhttp.open("GET", "/listfiles", false);
    xmlhttp.send();
    _("details").innerHTML = xmlhttp.responseText;
}

function showPreview(fileName) {
    /* _("img01").src = fileName;
    _("caption").innerHTML = fileName;
    _("myModal").style.display = "block"; */
    window.open(fileName);
}