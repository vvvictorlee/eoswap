Object.defineProperty(global, '__stack', {
    get: function () {
        var orig = Error.prepareStackTrace;
        Error.prepareStackTrace = function (_, stack) {
            return stack;
        };
        var err = new Error;
        Error.captureStackTrace(err, arguments.callee);
        var stack = err.stack;
        Error.prepareStackTrace = orig;
        return stack;
    }
});

Object.defineProperty(global, '__line', {
    get: function () {
        return __stack[1].getLineNumber();
    }
});

Object.defineProperty(global, '__function', {
    get: function () {
        return __stack[1].getFunctionName();
    }
});

// function foo() {
//     console.log(__line);
//     console.log(__function);
// }

// foo()


//前置通知
Function.prototype.before = function (func) {
    var that = this;
    args = [].slice.call(arguments, 1);
    return function () {
        //debugger
        if (func.apply(this, args) === false) {
            return false;
        }
        return that.apply(this, arguments);
    }
}

//后置通知
Function.prototype.after = function (func) {
    var that = this;
    args = [].slice.call(arguments, 1);
    return function () {
        var ret = that.apply(this, arguments);
        if (ret === false) {
            return false;
        }
        func.apply(this, args);
        return ret;
    }
}

//环绕型
Function.prototype.around = function (beforeFunc, afterFunc) {
    var that = this;
    return function () {
        return that.before(beforeFunc).after(afterFunc).apply(this, args);
    }
}

//捕获异常
Function.prototype.throwing = function (throwingFunc) {
    var that = this;
    args = [].slice.call(arguments, 1);
    return function () {
        try {
            return that.apply(this, arguments);
        } catch (e) {
            throwingFunc && throwingFunc.call(this, e, args);
        }
    }
}

function functionName(func) {
    // Match:
    // - ^          the beginning of the string
    // - function   the word 'function'
    // - \s+        at least some white space
    // - ([\w\$]+)  capture one or more valid JavaScript identifier characters
    // - \s*        optionally followed by white space (in theory there won't be any here,
    //              so if performance is an issue this can be omitted[1]
    // - \(         followed by an opening brace
    //
    var result = /([\w\$]+)\s*\(/.exec(func.toString())

    return result ? result[1] : '' // for an anonymous function there won't be a match
}

//将时间记录函数封装一下
function logTime(func) {
    return func1 = (function () {
        var d;
        return func.around(function () {
            d = +new Date();
            console.log(+new Date() - d, "==================", functionName(func), "===begin===============");
        }, function () {
            console.log(+new Date() - d, "==================", functionName(func), "===end===============");
        });
    })()
}

// module.exports = { logTime };


//像年轮一样将业务函数包裹，不会污染已有代码
// logTime(complexFunc)();

// logTime(otherComplexFunc)();

//运行结果：
// 1014
// 2024

//     console.log(__line);
//     console.log(__function);


// var a = {
//   b:function(){
//     console.log("this a.b()");
//   }
// }

function observe(obj) {
    for (var key in obj) {
        var defaultValue = obj[key];
        (function (k, dv) {
            Object.defineProperty(obj, k, {
                get: function () {
                    //console.log(dv);
                    //console.log(k + "'s function was invoked");
                    var funcStr = dv.toString();
                    var startIndex = funcStr.indexOf("(){");
                    var logic = funcStr.slice(startIndex + 3, funcStr.length - 1);
                    //console.log(logic);
                    var appendLogic = "var functionName = \"" + k + "\";";
                    appendLogic += "console.log(functionName);";
                    var finalLogic = appendLogic + logic;
                    var foo = new Function(finalLogic);
                    return foo;
                }
            });
        })(key, defaultValue);
    }
}

module.exports = { logTime, observe };

// observe(a);

// a.b();

