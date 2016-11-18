####################################################
# File Name: push_file.sh
# Author: YanWei
# Email: nameqiaohe@126.com
# Create Time: Wed 20 Jul 2016 06:05:58 PM CST
# Last Modified: Wed 20 Jul 2016 06:05:58 PM CST
####################################################
#!/bin/bash

git add $1

git commit -m "commit $1 : $2"

git push
