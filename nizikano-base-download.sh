#!/bin/bash

for a in `seq -w 01 23`
do wget 'http://app.nizikano-2d.jp/resource/audio.php?1461883136&id1=1&id2='$a -O se_$a.ogg
done

for a in `seq 1 8`
do wget 'http://app.nizikano-2d.jp/resource/audio.php?1461883136&id1=3&id2='$a -O bgm_$a.ogg
done
