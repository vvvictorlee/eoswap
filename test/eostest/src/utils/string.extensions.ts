interface Number {
  padZero(length: number):string;
thousandsSeperator(): String;
}

interface String {
  padZero(length: number):string;
}

String.prototype.padZero = function (length: number):string {
  var s: string = String(this);
  while (s.length < length) {
    s = '0' + s;
  }
  return s;
}

Number.prototype.padZero = function (length: number):string {
  return String(this).padZero(length)
}
Number.prototype.thousandsSeperator = function (): string {
    return Number(this).toString().replace(/\B(?=(\d{3})+(?!\d))/g, ',');
}