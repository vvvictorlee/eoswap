// import { SafeMath } from "../lib/SafeMath";
const Decimal = require('decimal.js');
// import {Decimal} from 'decimal.js';



interface Number {
    mul(b: number): number;

    div(b: number): number;

    divCeil(b: number): number;

    sub(b: number): number;

    add(b: number): number;

    sqrt(): number;
}

Number.prototype.mul = function (b: number): number {
    return Decimal(this).mul(b);
}
Number.prototype.div = function (b: number): number {
    return Decimal(this).div(b);
}

Number.prototype.sub = function (b: number): number {
    return Decimal(this).sub(b);
}
Number.prototype.add = function (b: number): number {
    return Decimal(this).add(b);
}
Number.prototype.sqrt = function (): number {
    return Decimal(this).sqrt();
}
Number.prototype.divCeil = function (b: number): number {
    let a:number = this.valueOf();
    let quotient: number = Decimal(a).div(b);
    let remainder: number = a - quotient * b;
    if (remainder > 0) {
        return quotient + 1;
    } else {
        return quotient;
    }
// return 1;//Decimal(this).div(b);
// return SafeMath.div(this,b);
}

// export { }; // this file needs to be a module


// (function () {
//     if (!String.prototype.isNullOrEmpty) {
//         String.prototype.isNullOrEmpty = function (this: string): boolean {
//             return !this;
//         };
//     }
//     if (!String.prototype.format) {
//         String.prototype.format = function () {
//             var args = arguments;
//             return this.replace(/{(\d+)}/g, function (match, number) {
//                 return typeof args[number] != 'undefined' ? args[number] : match;
//             });
//         };
//     }
//     if (!Array.prototype.firstOrDefault) { 
// Array.prototype.firstOrDefault = function (predicate: (item: any) => boolean) { for (var i = 0; i < (<Array<any>>this).length; i++) { let item = (<Array<any>>this)[i]; if (predicate(item)) { return item; } } return null; } } if (!Array.prototype.where) { Array.prototype.where = function (predicate: (item: any) => boolean) { let result = []; for (var i = 0; i < (<Array<any>>this).length; i++) { let item = (<Array<any>>this)[i]; if (predicate(item)) { result.push(item); } } return result; } } if (!Array.prototype.remove) { Array.prototype.remove = function (item: any): boolean { let index = (<Array<any>>this).indexOf(item); if (index >= 0) { (<Array<any>>this).splice(index, 1); return true; } return false; } } if (!Array.prototype.removeRange) { Array.prototype.removeRange = function (items: any[]): void { for (var i = 0; i < items.length; i++) { (<Array<any>>this).remove(items[i]); } } } if (!Array.prototype.add) { Array.prototype.add = function (item: any): void { (<Array<any>>this).push(item); } } if (!Array.prototype.addRange) { Array.prototype.addRange = function (items: any[]): void { for (var i = 0; i < items.length; i++) { (<Array<any>>this).push(items[i]); } } } if (!Array.prototype.orderBy) { Array.prototype.orderBy = function (propertyExpression: (item: any) => any) { let result = []; var compareFunction = (item1: any, item2: any): number => { if (propertyExpression(item1) > propertyExpression(item2)) return 1; if (propertyExpression(item2) > propertyExpression(item1)) return -1; return 0; } for (var i = 0; i < (<Array<any>>this).length; i++) { return (<Array<any>>this).sort(compareFunction); } return result; } } if (!Array.prototype.orderByDescending) { Array.prototype.orderByDescending = function (propertyExpression: (item: any) => any) { let result = []; var compareFunction = (item1: any, item2: any): number => { if (propertyExpression(item1) > propertyExpression(item2)) return -1; if (propertyExpression(item2) > propertyExpression(item1)) return 1; return 0; } for (var i = 0; i < (<Array<any>>this).length; i++) { return (<Array<any>>this).sort(compareFunction); } return result; } } if (!Array.prototype.orderByMany) { Array.prototype.orderByMany = function (propertyExpressions: [(item: any) => any]) { let result = []; var compareFunction = (item1: any, item2: any): number => { for (var i = 0; i < propertyExpressions.length; i++) { let propertyExpression = propertyExpressions[i]; if (propertyExpression(item1) > propertyExpression(item2)) return 1; if (propertyExpression(item2) > propertyExpression(item1)) return -1; } return 0; } for (var i = 0; i < (<Array<any>>this).length; i++) { return (<Array<any>>this).sort(compareFunction); } return result; } } if (!Array.prototype.orderByManyDescending) { Array.prototype.orderByManyDescending = function (propertyExpressions: [(item: any) => any]) { let result = []; var compareFunction = (item1: any, item2: any): number => { for (var i = 0; i < propertyExpressions.length; i++) { let propertyExpression = propertyExpressions[i]; if (propertyExpression(item1) > propertyExpression(item2)) return -1; if (propertyExpression(item2) > propertyExpression(item1)) return 1; } return 0; } for (var i = 0; i < (<Array<any>>this).length; i++) { return (<Array<any>>this).sort(compareFunction); } return result; } }
// })();

// export { }; 