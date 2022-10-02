function _(el) {
    return document.getElementById(el);
}
function __(cl) {
    return document.getElementsByClassName(cl);
}

var uploadForm;
var currentDir = "/";

window.addEventListener('load', function () {
    _('listFiles').addEventListener('click', () => {
        currentDir = "/";
        listFiles(currentDir);
    });
    listFiles("/");
    fetch('uploadForm.html').then(res => res.text()).then(data => uploadForm = data);
    fetch('/cardinfo').then(res => res.json()).then(data => {
        _("cardinfo").max = data['max'];
        _("cardinfo").value = data['val'];
        _("usedsd").textContent = data['usedsd'];
        _("freesd").textContent = data['freesd'];
    })
    _("usedsd").textContent = _("freesd").textContent = "...";
});

function listFiles(dirPath) {
    currentDir = dirPath;
    _('details').textContent = 'loading...';
    _('detailsheader').textContent = '';
    _('mkdirField').textContent = currentDir + '/';
    const mkdirText = _('mkdirField').appendChild(document.createElement('input'));
    mkdirText.placeholder = 'new folder name';
    const mkdirBtn = _('mkdirField').appendChild(document.createElement('button'));
    mkdirBtn.innerHTML = '&#xe8f4';
    mkdirBtn.addEventListener('click', () => {
        fileAction(currentDir, 'mkdir', null, mkdirText.value);
    });
    const url = new URL('/listfiles', window.location.origin);
    url.searchParams.append('path', dirPath);
    fetch(url).then(res => res.json()).then(data => {
        const fileListTable = document.createElement('table');
        const fileListHead = fileListTable.appendChild(document.createElement('thead')).appendChild(document.createElement('tr'));
        fileListHead.append(document.createElement('th'));
        const fileListHeadName = fileListHead.appendChild(document.createElement('th'));
        fileListHeadName.textContent = 'Name';
        fileListHeadName.scope = 'col';
        const fileListHeadSize = fileListHead.appendChild(document.createElement('th'));
        fileListHeadSize.textContent = 'Size';
        fileListHeadSize.scope = 'col';
        const fileListBody = fileListTable.appendChild(document.createElement('tbody'));
        for (const fileKey in data) {
            const fileListBodyRow = fileListBody.appendChild(document.createElement('tr'));
            fileListBodyRow.classList.add('fileLn');
            fileListBodyRow.classList.add('clickable');
            fileListBodyRow.setAttribute('fileName', fileKey);
            fileListBodyRow.setAttribute('filePath', data[fileKey]['path']);
            const fileListBodyRowIcon = fileListBodyRow.appendChild(document.createElement('th'));
            fileListBodyRowIcon.innerHTML = fileIcon(fileKey);
            fileListBodyRowIcon.classList.add('fIcon');
            fileListBodyRowIcon.classList.add('msIcon');
            fileListBodyRowIcon.scope = 'ln';
            fileListBodyRowIcon.id = fileKey;
            const fileListBodyRowName = fileListBodyRow.appendChild(document.createElement('td')).appendChild(document.createElement('span'));
            fileListBodyRowName.textContent = fileKey;
            const fileListBodyRowSize = fileListBodyRow.appendChild(document.createElement('td'));
            fileListBodyRowSize.textContent = data[fileKey]['size'];
            fileListBodyRowSize.classList.add('fSizeVal');
            fileListBodyRowSize.title = data[fileKey]['adjustSize'];
            const fileListBodyRowOps = fileListBodyRow.appendChild(document.createElement('td'));
            const fileListBodyRowOpsDown = fileListBodyRowOps.appendChild(document.createElement('td')).appendChild(document.createElement('button'));
            fileListBodyRowOpsDown.innerHTML = '&#xe896;';
            fileListBodyRowOpsDown.className = 'down';
            const fileListBodyRowOpsDel = fileListBodyRowOps.appendChild(document.createElement('td')).appendChild(document.createElement('button'));
            fileListBodyRowOpsDel.innerHTML = '&#xe74d;';
            fileListBodyRowOpsDel.className = 'del';
            fileListBodyRowOpsDown.parentElement.className = fileListBodyRowOpsDel.parentElement.className = 'fileOpsCell';
            if (!data[fileKey]['isDir']) {
                fileListBodyRow.classList.add('isFile');
                fileListBodyRowName.classList.add('renameFile');
            } else {
                fileListBodyRow.classList.add('dir');
                fileListBodyRowOpsDown.disabled = true;
            }
        }
        _('details').replaceChildren(fileListTable);
        Array.from(__('fileLn isFile')).forEach(fileOpen => fileOpen.addEventListener('click', () => { window.open(fileOpen.getAttribute('filePath'), '_blank'); }));
        Array.from(__('down')).forEach(fileDown => fileDown.addEventListener('click', (e) => {
            e.stopPropagation();
            fileAction(e.target.parentElement.parentElement.parentElement.getAttribute('filePath'), 'download', e.target.parentElement.parentElement.parentElement.getAttribute('fileName'), null);
        }));
        Array.from(__('del')).forEach(fileDel => fileDel.addEventListener('click', (e) => {
            e.stopPropagation();
            fileAction(e.target.parentElement.parentElement.parentElement.getAttribute('filePath'), 'delete', null, null);
        }));
        Array.from(__('renameFile')).forEach(fileRen => fileRen.addEventListener('click', showRenameForm));
        Array.from(__('dir')).forEach(dirOpen => dirOpen.addEventListener('click', () => {
            currentDir = dirOpen.getAttribute('filePath');
            listFiles(currentDir);
        }));
    });
}

function showRenameForm(event) {
    event.stopPropagation();
    const renameForm = document.createElement('form');
    const renameFormText = renameForm.appendChild(document.createElement('input'));
    renameFormText.value = event.target.parentElement.parentElement.getAttribute('fileName');
    renameFormText.autofocus = true;
    renameFormText.id = 'newName';
    renameFormText.addEventListener('click', (e) => { e.stopPropagation(); });
    renameFormText.addEventListener('focusout', (e) => {
        e.target.parentElement.parentElement.textContent = e.target.parentElement.parentElement.parentElement.parentElement.getAttribute('fileName');
    });
    const renameFormBtn = renameForm.appendChild(document.createElement('button'));
    renameFormBtn.innerHTML = '&#xe73e;';
    renameFormBtn.addEventListener('mousedown', (e) => {
        e.preventDefault();
        fileAction(e.target.parentElement.parentElement.parentElement.parentElement.getAttribute('filePath'), 'rename', null, e.target.parentElement.parentElement.parentElement.parentElement.getAttribute('filePath').replace(e.target.parentElement.parentElement.parentElement.parentElement.getAttribute('fileName'), _('newName').value));
    });
    event.target.replaceChildren(renameForm);
}

function fileAction(path, action, name, newPath) {
    const data = new Object();
    data.path = path;
    data.action = action;
    data.newPath = newPath;
    let req = fetch('/file', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', },
        body: JSON.stringify(data),
    });
    if (action == 'download')
        req.then(res => res.blob()).then(data => {
            const tempDown = document.createElement('a');
            tempDown.href = URL.createObjectURL(data);
            tempDown.download = name;
            tempDown.click();
        });
    else {
        req.then(res => res.json()).then(data => {
            _('status').textContent = (data['success'] ? 'Success: ' : 'Failed: ') + data['action'] + ' ' + data['target'];
        }).then(listFiles(currentDir));
    }
}

function showUploadForm() {
    const uploadTitle = document.createElement('h3');
    uploadTitle.textContent = 'Upload Files';
    _("detailsheader").replaceChildren(uploadTitle);
    _("status").textContent = "";
    _("details").innerHTML = uploadForm;
    _('uploadConfirm').addEventListener('click', (e) => {
        e.preventDefault();
        uploadFile();
    });
    _('file').addEventListener('change', uploadFormEvent);
    _('fileForm').addEventListener('reset', uploadFormEvent);
}

function uploadFormEvent(e) {
    let total_size = 0;
    const upFileInfo = document.createDocumentFragment();
    const table_file_head = upFileInfo.appendChild(document.createElement('tr'));
    table_file_head.append(document.createElement('th'));
    table_file_head.appendChild(document.createElement('th')).textContent = 'Name';
    table_file_head.appendChild(document.createElement('th')).textContent = 'Size (bytes)';
    Array.from(e.target.files).forEach(file => {
        const table_file_row = upFileInfo.appendChild(document.createElement('tr'));
        const tableFileRowIcon = table_file_row.appendChild(document.createElement('th'));
        tableFileRowIcon.innerHTML = fileIcon(file.name);
        tableFileRowIcon.classList.add('msIcon');
        tableFileRowIcon.classList.add('fIcon');
        table_file_row.appendChild(document.createElement('td')).textContent = file.name;
        table_file_row.appendChild(document.createElement('td')).textContent = file.size;
        total_size += file.size;
    });
    const table_file_total = upFileInfo.appendChild(document.createElement('tr'));
    table_file_total.append(document.createElement('th'));
    table_file_total.appendChild(document.createElement('th')).textContent = e.target.files.length;
    table_file_total.appendChild(document.createElement('th')).textContent = total_size;
    _('pendingFiles').replaceChildren(upFileInfo);
}

function uploadFile() {
    //var formdata = new FormData(_('fileForm'));
    //Array.from(_("file").files).forEach(file => formdata.append("files", file));
    var xhr = new XMLHttpRequest();
    xhr.upload.addEventListener("progress", progressHandler, false);
    xhr.addEventListener("load", completeHandler, false);
    xhr.addEventListener("error", errorHandler, false);
    xhr.addEventListener("abort", abortHandler, false);
    const url = new URL('/', window.location.origin);
    url.searchParams.append('path', currentDir + '/');
    xhr.open("POST", url);
    xhr.send(new FormData(_('fileForm')));
}
function progressHandler(e) {
    _('loaded_n_total').textContent = 'Uploaded ' + e.loaded + '/' + e.total + ' bytes';
    _('progressBar').hidden = false;
    _('progressBar').max = e.total;
    _('progressBar').value = e.loaded;
    _('status').textContent = Math.round((e.loaded / e.total) * 100) + '% uploaded... please wait';
    if (e.loaded >= e.total)
        _('status').textContent = 'Please wait, writing file to filesystem';
}
function completeHandler(event) {
    _("status").textContent = "Upload Complete";
    _("progressBar").value = 0;
    _("status").textContent = "File Uploaded";
    listFiles(currentDir);
}
function errorHandler(event) {
    _("status").textContent = "Upload Failed";
}
function abortHandler(event) {
    _("status").textContent = "inUpload Aborted";
}

function fileIcon(filename) {
    switch (filename.split('.').pop()) {
        case 'jpg':
        case 'jpeg':
        case 'png':
        case 'gif':
        case 'ico':
        case 'webp':
            return '&#xe8b9;';
        case 'mp4':
        case 'MP4':
        case 'mov':
        case 'webm':
            return '&#xe714;';
        case 'mp3':
            return '&#xe8d6;';
        case 'js':
        case 'css':
        case 'html':
        case 'bin':
            return '&#xe943;';
        case 'pdf':
            return '&#xea90;';
        case 'docx':
        case 'doc':
            return '&#xe8a5;';
        case 'ttf':
            return '&#xe8d2;';
        case 'json':
            return '&#xe713;';
        case 'zip':
        case 'rar':
        case '7z':
            return '&#xf012'

        default:
            return '&#xe8b7;';
    }
}