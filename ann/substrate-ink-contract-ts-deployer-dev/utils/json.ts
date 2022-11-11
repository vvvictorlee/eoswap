const fs = require('fs');

 export  async function savejson(key: String, value: String, section: String,jsonfile:String) {
    let json: any = fs.readFileSync(jsonfile);

    if (json == undefined) {
        json = {};
    } else {
        json = JSON.parse(json);
    }
    if (json[section.toString()] == undefined) {
        json[section.toString()] = {}
    }
    json[section.toString()][key.toString()] = value;
    fs.writeFileSync(jsonfile, JSON.stringify(json));
}
  export async function getjson(key: String, section: String,jsonfile:String) {
    let json: any = fs.readFileSync(jsonfile);

    if (json == undefined) {
        return "";
    }
    json = JSON.parse(json);

    if (json[section.toString()] == undefined) {
        return ""
    }
    return json[section.toString()][key.toString()];

}

//  export default {savejson,getjson}