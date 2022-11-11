const { execCmd } = require("./cmd");
const { savejson, getjson } = require("./json");

const url = " --url ws://127.0.0.1:9944";//" --url ws://192.168.1.106:9944"

export async function upload(acc: String, path: String, key: String, jsonfile: String) {
    let output = ""
    try {
        output = await execCmd('cargo contract upload --suri //' + acc + url, "Code hash", path);
        let n = output.indexOf("\n");
        if (-1 != n) {
            output = output.slice(0, n);
        }
        savejson(key, output, "hash", jsonfile);
    } catch (error) {
        console.error(acc, path, "error=", error);
    }
    return output;
}
export async function instantiate(acc: String, args: String, path: String, key: String, jsonfile: String) {
    console.log("==================")
    console.log('cargo contract instantiate --constructor new ' + (args == undefined || args == "" ? "" : " --args " + args) + ' --suri //' + acc +" --manifest-path "+path+"/Cargo.toml" ," Code hash ",path);
    let output = "";
    try {
        // let output = await execCmd('cargo contract instantiate --constructor new ' + (args == undefined || args == "" ? "" : " --args " + args) + ' --suri //' + acc +" --manifest-path "+path+"/Cargo.toml" ,"Contract",path);
        // output = await execCmd('cargo contract instantiate --constructor new ' + (args == undefined || args == "" ? "" : " --args " + args) + ' --suri //' + acc + url, " Code hash ", path);        
        let output = await execCmd('cargo contract instantiate --constructor new ' + (args == undefined || args == "" ? "" : " --args " + args) + ' --suri //' + acc +" --manifest-path "+path+"/Cargo.toml" ," Code hash ",path);

        let keyword = " Contract ";
        let codehash = output.substring(0, output.indexOf(keyword)).trim();
        let n = codehash.indexOf("\n");
        if (-1 != n) {
            codehash = codehash.slice(0, n);
        }
        let address = output.substring(output.indexOf(keyword) + keyword.length).trim();
        n = address.indexOf("\n");
        if (-1 != n) {
            address = address.slice(0, n);
        }
        savejson(key, codehash, "hash", jsonfile);
        savejson(key, address, "address", jsonfile);
    } catch (error) {
        console.error(acc, path, "error=", error);
    }
    return output;
}
