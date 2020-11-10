var fs = require('fs');

module.exports = {
    loadJson,
    writeFile,
    writeSqlFile,
    checkPath
};

async function loadJson(fileName) {
    return JSON.parse(fs.readFileSync(fileName));
}

async function writeFile(fileName, json) {
    fs.writeFile(fileName, JSON.stringify(json), function (error) {
        if (error)
            console.log(error);
    });
}

function writeSqlFile(fileName, sql) {
    fs.writeFile(fileName, sql, function (error) {
        if (error) console.log(error);
    });
}

// 检查并创建路径
function checkPath(path) {
    if (!fs.existsSync(path)) {
        fs.mkdirSync(path);
    }
}
