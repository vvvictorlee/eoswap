
// import fs from 'fs';
// import path from 'path';

// import { Json } from "@polkadot/types"

// var fs = require("fs")
const configFile = `../abi/art.json`
const nameFile = `../abi/abi2name.json`
export function savejson(json, file) {
    try {
        // const cmpPath = path.join(__dirname, file);

        // if (fs.existsSync(cmpPath)) {
        //     fs.writeFileSync(cmpPath, JSON.stringify(json, null, 2), { flag: 'w' });
        // } else {
        //     console.error("===savejson=not exists==", cmpPath)
        // }
        localStorage.saveItem("json",JSON.stringify(json));
    } catch (error) {
        console.error(error,file)
    }


}

export function readjson(file) {
    try {
        // const cmpPath = path.join(__dirname, file);

        // if (!fs.existsSync(cmpPath)) {
        //     return {};
        // }

        // return JSON.parse(fs.readFileSync(cmpPath, 'utf8'));
        let json=localStorage.getItem("json");
        if (json==null||json===undefined){
        return {}}
       return  JSON.parse(json)
    } catch (error) {
        console.error(error,file)
    }
}

export function getValue(key) {
    const json = readjson(configFile);
    if (json["address"][key] === undefined) {
        return 0
    }
    return json["address"][key]
}
export function saveValue(key, value) {
    putKV("address", key, value)
}

export function getNames() {
    const json = readjson(nameFile);
    if (json === undefined) {
        return {}
    }
    return json
}

export function getAddresses() {
    const json = readjson(configFile);
    if (json["address"] === undefined) {
        return {}
    }
    return json["address"]
}

export function setIsInitilized() {
    const json = readjson(configFile);
    json["IsInitilized"] = "true";
    savejson(json, configFile);
}

export function getIsInitilized() {
    const json = readjson(configFile);
    if (json["IsInitilized"] === undefined) {
        return false
    }
    return json["IsInitilized"]==="true"
}


export function putKV(key, secondkey, value) {
    const json = readjson(configFile);
    json[key][secondkey] = value;
    savejson(json, configFile);
}