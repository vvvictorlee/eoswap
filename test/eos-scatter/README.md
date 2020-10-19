# hello-eos-scatter
# 准备工作
## 安装scatter
安装chrome scatter插件，并已绑定一个账号，具体参考
https://blog.csdn.net/ITleaks/article/details/83409553

# hello智能合约 scatter前端
hello-eos-scatter是基于React和Scatterjs的网页前端，能够非常方便的使用scatter连接用户，并执行
eos的智能合约。
大致代码如下
## 连接scatter
```
async connect(){
  //change name 'hello-scatter' to your application's name
  this.connected = await ScatterJS.scatter.connect('hello-scatter')
  console.log(this.connected);
}
```
## 获取账号信息
```
let result = await ScatterJS.scatter.getIdentity({accounts:[this.network]})
this.currentAccount = result.accounts[0];
console.log("login success,", this.currentAccount)
```
## 运行
```
npm install
npm -g install react-scripts
npm start
```
