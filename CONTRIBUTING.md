## Some suggestions about git usage

1. Create a issue firstly for any bug or feature and let commit link to it
2. use `git rebase` to merge code, commit message like `Merge pull request #5 from kernelrookie/grub` is annoying and distractive
    1. what's [`git rebase`](https://www.atlassian.com/git/tutorials/rewriting-history/git-rebase)
    2. [how to use it](https://stackoverflow.com/questions/16358418/how-to-avoid-merge-commit-hell-on-github-bitbucket)
3. Don't commit code together with doc. Only doc or code.
4. 为了一次 commit 的语义完整，每次提交都需要通过 ci, 如果没有通过，应该提交补丁，并且 squash 两者
    - 如果不小心 push 到 remote 了, 请不要使用 force push
    - [ ] github action 通过 act 可以本地运行，但是 act 使用 docker, 而 docker 内没有代理，这导致本地 ci 很慢，所以暂时使用 pre-commit 做检查的。

## how to collaborate
1. 新手更加应该参与到 review
2. 如何让 reviewer 愉快的 review 你的代码[^1]

## 使用 git pre-commit
我希望尽量不要出现 typo fix, code format 之类的提交，这对于 reviewer 和 commiter 都是毫无意义的精力浪费。

pre-commit 会在进行 commit 之前执行一些脚本做出检查，如果检查不通过，
就会提示出来, GitKraken[^2] 的教程图文并茂，非常不错。

实际上，很多 hook 都是通用的，于是乎就有了 pre-commit 项目，可以安装一些常用的 hook, 具体配置在 .pre-commit-config.yaml 中

我希望在提交代码之前保证代码都是被 format 过的，所以:
https://ortogonal.github.io/cpp/git-clang-format/
```sh
# Test clang-format
clangformatout=$(git clang-format --diff -q)

# Redirect output to stderr.
exec 1>&2

if [ "$clangformatout" != "" ]
then
    echo "Format error!"
    echo "Use git clang-format"
    exit 1
fi
```

[^1]: https://mtlynch.io/code-review-love/#12-award-all-ties-to-your-reviewer
[^2]: https://support.gitkraken.com/working-with-repositories/githooksexample/
[^3]: https://pre-commit.com/
[^4]: https://stackoverflow.com/questions/5667884/how-to-squash-commits-in-git-after-they-have-been-pushed
