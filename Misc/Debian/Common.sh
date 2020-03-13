#!/bin/bash


export col_green=$(tput setaf 2)
export col_red=$(tput setaf 1)
export col_normal=$(tput sgr0)

function Ok {
  printf "${col_green}Ok${col_normal}\n"
}

function Fail {
  printf "${col_red}Fail${col_normal}\n"
  exit 1
}

function Fatal {
  read -p "Confirm ${col_red}FAIL${col_normal}"
  exit 1
}

function Success {
  read -p "Confirm ${col_green}SUCCESS${col_normal}"
  exit 0
}

export -f Ok
export -f Fail
export -f Fatal
export -f Success

set -o pipefail
