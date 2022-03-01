#!/bin/bash
sudo npm install -g --save-dev @commitlint/{config-conventional,cli}
sudo npm install -g husky --save-dev
npx husky install
npx husky add .husky/commit-msg 'npx --no-install commitlint --edit "$1"'

sudo pip3 install pre-commit
