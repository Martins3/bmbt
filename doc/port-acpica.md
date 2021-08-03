# how integrate acpica

1. 代码拷贝
```sh
git clone ...
cd generate/linux
./gen-repo.sh
```

2. 修改头文件
3. 修改 Makefile

我找到了好几个 mini kernel 的存在，目前来说，几乎没有人会
将整个 acpica 包含进去，我们应该好好调查一下，真的有必要吗?
