function _(el) {
    return document.getElementById(el);
}
function __(cl) {
    return document.getElementsByClassName(cl);
}

var uploadForm;

window.onload = function () {
    listFilesButton();
    fetch('uploadForm.html').then(res => res.text()).then(data => uploadForm = data);
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
        const fileListTable = document.createDocumentFragment().appendChild(document.createElement('table'));
        const fileListHead = fileListTable.appendChild(document.createElement('thead')).appendChild(document.createElement('tr'));
        const fileListHeadName = fileListHead.appendChild(document.createElement('th'));
        fileListHeadName.textContent = 'Name';
        fileListHeadName.scope = 'col';
        const fileListHeadSize = fileListHead.appendChild(document.createElement('th'));
        fileListHeadSize.textContent = 'Size';
        fileListHeadSize.scope = 'col';
        const fileListBody = fileListTable.appendChild(document.createElement('tbody'));
        for (const fileKey in data) {
            const fileListBodyRow = fileListBody.appendChild(document.createElement('tr'));
            fileListBodyRow.className = 'fileLn';
            fileListBodyRow.id = fileKey;
            const fileListBodyRowIcon = fileListBodyRow.appendChild(document.createElement('td'));
            fileListBodyRowIcon.innerHTML = fileIcon(fileKey);
            fileListBodyRowIcon.classList.add('fIcon');
            fileListBodyRowIcon.classList.add('msIcon');
            fileListBodyRowIcon.id = fileKey;
            fileListBodyRow.appendChild(document.createElement('td')).textContent = fileKey;
            fileListBodyRow.appendChild(document.createElement('td')).textContent = data[fileKey]['size'];
            if (!data[fileKey]['isDir']) {
                const fileListBodyRowOps = fileListBodyRow.appendChild(document.createElement('td'));
                const fileListBodyRowOpsDown = fileListBodyRowOps.appendChild(document.createElement('button'));
                fileListBodyRowOpsDown.innerHTML = '&#xe896;';
                fileListBodyRowOpsDown.className = 'down';
                const fileListBodyRowOpsDel = fileListBodyRowOps.appendChild(document.createElement('button'));
                fileListBodyRowOpsDel.innerHTML = '&#xe74d;';
                fileListBodyRowOpsDel.className = 'del';
            }
            /*             
            let appStr = '<tr id="fileLn">' +
                            '<td class="fIcon" onclick="window.open(\'' + data[fileKey]['view'] + '\')">' + fileIcon(fileKey) + '</td>' +
                            '<td onclick="window.open(\'' + data[fileKey]['view'] + '\')">' + fileKey + '</td><td>' + data[fileKey]['size'] + '</td>';
                        if (!data[fileKey]['isDir'])
                            appStr += "<td>" + '<button id="down"' + 'onclick="downloadDeleteButton(\'' + fileKey + '\', \'download\')">&#xe896;</button>' + "</td><td>" + '<button id="del" onclick="downloadDeleteButton(\'' + fileKey + '\', \'delete\')">&#xe74d;</button>' + "</td>";
                        appStr += "</tr>";
                        _('fList').insertAdjacentHTML('beforeend', appStr);
             */
        }
        _('details').replaceChildren(fileListTable);
        Array.from(__('fIcon')).forEach(fileOpen => fileOpen.addEventListener('click', function () { window.open(fileOpen.id); }));
        Array.from(__('down')).forEach(fileDown => fileDown.addEventListener('click', function () {
            const url = new URL('/file', window.location.origin);
            url.searchParams.append('name', fileDown.parentElement.parentElement.id);
            url.searchParams.append('action', 'download');
            _("status").innerHTML = "";
            window.location.replace(url);
        }));
        Array.from(__('del')).forEach(fileDel => fileDel.addEventListener('click', function () {
            const url = new URL('/file', window.location.origin);
            url.searchParams.append('name', fileDel.parentElement.parentElement.id);
            url.searchParams.append('action', 'delete');
            fetch(url).then(res => res.text()).then(data => _("status").innerHTML = data).then(listFilesButton());
        }));
    })
}
/* 
function downloadDeleteButton(filename, action) {
    var urltocall = "/file?name=" + filename + "&action=" + action;
    if (action == "delete")
        fetch(urltocall).then(res => res.text()).then(data => _("status").innerHTML = data).then(listFilesButton());
    if (action == "download") {
        _("status").innerHTML = "";
        window.location.replace(urltocall);
    }
}
 */
function showUploadForm() {
    const uploadTitle = document.createElement('h3');
    uploadTitle.textContent = 'Upload Files';
    _("detailsheader").replaceChildren(uploadTitle);
    _("status").innerHTML = "";
    _("details").innerHTML = uploadForm;
    _('uploadConfirm').addEventListener('click', (e) => {
        e.preventDefault();
        uploadFile();
    });
    _('file').addEventListener('change', uploadFormEvent);
    _('file').addEventListener('reset', uploadFormEvent);
}

function uploadFormEvent(e) {
    let total_size = 0;
    const upFileInfo = document.createDocumentFragment();
    const table_file_head = upFileInfo.appendChild(document.createElement('tr'));
    table_file_head.appendChild(document.createElement('th')).textContent = 'Name';
    table_file_head.appendChild(document.createElement('th')).textContent = 'Size (bytes)';
    Array.from(e.target.files).forEach(file => {
        const table_file_row = upFileInfo.appendChild(document.createElement('tr'));
        table_file_row.appendChild(document.createElement('td')).textContent = file.name;
        table_file_row.appendChild(document.createElement('td')).textContent = file.size;
        total_size += file.size;
    });
    const table_file_total = upFileInfo.appendChild(document.createElement('tr'));
    table_file_total.appendChild(document.createElement('td')).textContent = e.target.files.length;
    table_file_total.appendChild(document.createElement('td')).textContent = total_size;
    _('pendingFiles').replaceChildren(upFileInfo);
}

function uploadFile() {
    var formdata = new FormData();
    Array.from(_("file").files).forEach(file => formdata.append("files", file));
    var ajax = new XMLHttpRequest();
    ajax.upload.addEventListener("progress", progressHandler, false);
    ajax.addEventListener("load", completeHandler, false);
    ajax.addEventListener("error", errorHandler, false);
    ajax.addEventListener("abort", abortHandler, false);
    ajax.open("POST", "/");
    ajax.send(formdata);
}
function progressHandler(event) {
    _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
    var percent = (event.loaded / event.total) * 100;
    _('progressBar').hidden = false;
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
            return '&#xe8d2;';
        case 'json':
            return '&#xe713;';

        default:
            return '&#xe8b7;';
    }
}