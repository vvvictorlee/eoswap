#EOSWAPX
###文件结构
```
.
├── build.sh
├── contracts
│   ├── CMakeLists.txt
│   ├── eoswap
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   ├── common
│   │   │   │   └── BType.hpp
│   │   │   ├── eoswap
│   │   │   │   ├── BColor.hpp
│   │   │   │   ├── BConst.hpp
│   │   │   │   ├── BFactory.hpp
│   │   │   │   ├── BMath.hpp
│   │   │   │   ├── BNum.hpp
│   │   │   │   ├── BPool.hpp
│   │   │   │   └── BToken.hpp
│   │   │   └── storage
│   │   │       ├── BFactoryTable.hpp
│   │   │       ├── BPoolTable.hpp
│   │   │       └── BTokenTable.hpp
│   │   ├── ricardian
│   │   └── src
│   │       └── eoswap.cpp
├── mybuild.sh
├── mytest.sh
├── scripts
│   └── helper.sh
└── tests
    ├── CMakeLists.txt
    ├── contracts.hpp.in
    ├── eoswap_tests.cpp
    ├── main.cpp
```
#####主要文件 
* contracts/eoswap 合约代码
* tests/eoswap_tests.cpp 单元测试用例代码
* contracts.hpp.in 编译合约abi,wasm用于单元测试
* mybuild.sh 编译合约  添加自定义参数
* mytest.sh 执行eoswap单元测试


#####RoxeChain编译前提条件
- 编译系统合约的开发环境


#####RoxeChain说明
1. 需要把相关文件放到对应位置
2. 按RoxeChain名称修改相应引用头文件
