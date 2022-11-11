const util = require('util');
const exec = util.promisify(require('child_process').exec);

export async function execCmd(cmd: String, keyword: String, path: String) {
    let { err, stdout, stderr } = await exec(cmd, { cwd: path });
    if (err) {
        console.error(err);
        // return;
    }
    console.log("========stdout==========", stdout, "========stderr==========", stderr);
    return stdout.substring(stdout.indexOf(keyword) + keyword.length).trim();
}

export async function execSysCmd(cmd: String, path: String) {
    let { err, stdout, stderr } = await exec(cmd, { cwd: path });
    if (err) {
        console.error(err);
        // return;
    }
    console.log("========stdout==========", stdout, "========stderr==========", stderr);
    return stdout.trim();
}

//   export default {execCmd}