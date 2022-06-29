## Some suggestions about git usage
- use `git rebase` to merge code, commit message like `Merge pull request #5 from kernelrookie/grub` is annoying and distractive
    1. what's [`git rebase`](https://www.atlassian.com/git/tutorials/rewriting-history/git-rebase)
    2. [how to use it](https://stackoverflow.com/questions/16358418/how-to-avoid-merge-commit-hell-on-github-bitbucket)

## 关于代码的设计想法
1. 新手更加应该参与到 review
2. 如何让 reviewer 愉快的 review 你的代码[^1]
3. 一个优雅的 commit[^5] 应该是什么样子的

## 使用 [pre-commit](https://pre-commit.com/) 来进行常规检查
> If you want to manually run all pre-commit hooks on a repository, run
>  `pre-commit run --all-files`. To run individual hooks use `pre-commit run <hook_id>`.

我希望尽量不要出现 typo fix, code format 之类的提交，这对于 reviewer 和 commiter 都是毫无意义的精力浪费。

pre-commit 会在进行 commit 之前执行一些脚本做出检查，如果检查不通过，就会提示出来, GitKraken[^2] 的教程图文并茂，非常不错。

实际上，很多 hook 都是通用的，于是乎就有了 pre-commit 项目，可以安装一些常用的 hook, 具体配置在 .pre-commit-config.yaml 中
而且，我还添加了一些新的规则:
- format-code.sh : 在提交代码之前保证代码都是被 format 过的，用 git blame 看到的信息如果是 code format, 那相当于什么都没说。
- lint-md.sh :  中文文档需要按照规范书写
- separate-doc-code.sh : 代码的修改和文档的修改不在在一个 commit 中提交
- code-test.sh : 每次提交需要保证通过测试
- [ ] non-ascii-comment.sh : 不要在代码中携带中文注释，最好的代码是没有注释的，如果需要，那么就使用英文，不过表达不清楚，那么就写成一个 blog

这是正常的 commit 之前的 log，如果有检查不过，那么就会提前 exit
```c
[WARNING] Unstaged files detected.
[INFO] Stashing unstaged files to /home/maritns3/.cache/pre-commit/patch1633784307-412875.
Trim Trailing Whitespace.................................................Passed
Fix End of Files.........................................................Passed
Check Yaml...........................................(no files to check)Skipped
Check for added large files..............................................Passed
Check for merge conflicts................................................Passed
Don't commit to branch...................................................Passed
format code..............................................................Passed
lint-md..................................................................Passed
code test................................................................Passed
separate-doc-code........................................................Passed
[INFO] Restored changes from /home/maritns3/.cache/pre-commit/patch1633784307-412875.
[dev 589366f] refactor(cpu): cpu_create is useless, remove it
 1 file changed, 6 insertions(+), 5 deletions(-)
```

## 使用 [commitlint](https://github.com/conventional-changelog/commitlint) 来检查 commit
使用 npm 的安装
```sh
sudo npm install -g --save-dev @commitlint/{config-conventional,cli}
sudo npm install -g husky --save-dev
npx husky install
npx husky add .husky/commit-msg 'npx --no-install commitlint --edit "$1"'
```
然后 commit 的规则为:
```txt
type(scope?): subject  #scope is optional; multiple scopes are supported (current delimiter options: "/", "\" and ",")
```
规则具体含义参考[这里](https://github.com/conventional-changelog/commitlint/blob/master/%40commitlint/config-conventional/index.js)

husky 有点烦人的地方在于其会修改 `git config core.hooksPath`[^6] 的路径为 `.husky`，这会导致 .git/hooks/pre-commit 失效
暂时的方法将 `.git/hooks/pre-commit` 拷贝到 `.husky` 中，所以无需使用 `pre-commit install` 了。

## 记录一下疑惑
- 一般来说，使用 rebase and merge
  - 但是如果 local 有 commit 没有推上去，就需要手动 merge 了

[^1]: https://mtlynch.io/code-review-love/#12-award-all-ties-to-your-reviewer
[^2]: https://support.gitkraken.com/working-with-repositories/githooksexample/
[^3]: https://pre-commit.com/
[^4]: https://stackoverflow.com/questions/5667884/how-to-squash-commits-in-git-after-they-have-been-pushed
[^5]: https://dhwthompson.com/2019/my-favourite-git-commit
[^6]: https://stackoverflow.com/questions/56068823/if-we-change-git-config-core-hookspath-how-to-revert-back-to-default
