
import fs from 'fs';
import path from 'path';

const configFile = `../abi/art.json`
const nameFile = `../abi/abi2name.json`
export function savejson(json: any, file: any) {
    try {
        const cmpPath = path.join(__dirname, file);

        if (fs.existsSync(cmpPath)) {
            fs.writeFileSync(cmpPath, JSON.stringify(json, null, 2), { flag: 'w' });
        } else {
            console.error("===savejson=not exists==", cmpPath)
        }
    } catch (error) {
        console.error(error)
    }


}

export function readjson(file: any): any {
    try {
        const cmpPath = path.join(__dirname, file);

        if (!fs.existsSync(cmpPath)) {
            return {};
        }

        return JSON.parse(fs.readFileSync(cmpPath, 'utf8'));

    } catch (error) {
        console.error(error)
    }
}

export function getValue(key: any) {
    const json = readjson(configFile);
    if (json["address"][key] == undefined) {
        return 0
    }
    return json["address"][key]
}
export function saveValue(key: any, value: any) {
    putKV("address", key, value)
}

export function getNames() {
    const json = readjson(nameFile);
    if (json == undefined) {
        return {}
    }
    return json
}

export function getAddresses() {
    const json = readjson(configFile);
    if (json["address"] == undefined) {
        return {}
    }
    return json["address"]
}

export function putKV(key: any, secondkey: any, value: any): any {
    const json = readjson(configFile);
    json[key][secondkey] = value;
    savejson(json, configFile);
}