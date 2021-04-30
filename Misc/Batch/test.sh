#!/bin/bash

export ShareDir=/tmp
export Version=$(<"$ShareDir"/Ver)
export pth=abc$Version

for fileName in *.sh; do
  baseName=${fileName/.sh/}
  openssl dgst -sha256 -sign nex_batch_private.pem -out ${baseName}.sgn ${baseName}.sh
done

for fileName in *.7z; do
  baseName=${fileName/.7z/}
  openssl dgst -sha256 -sign nex_batch_private.pem -out ${baseName}.sgn ${baseName}.7z
done
