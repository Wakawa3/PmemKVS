PmemKVSテスト

構造
/mnt/pmem0/dataでデータの中身を管理
struct KVstruct型のデータが配列のように連続して配置されている

/mnt/pmem0/numでデータの個数を管理

使い方
gcc PmemKVS.c -lpmem -lpmemobj
でコンパイル

a.out -w <key> <int>
で書き込み
初めて書き込むと/mnt/pmem0ディレクトリにdataとnumというファイルが生成される
書き込むとnumに記録されている数字がインクリメントされる

a.out -r <key>
で読み込み

a.out -ra
で全要素読み込み

a.out -d <key>
で要素削除
ある要素を削除すると最後尾のデータに置き換えられる
削除するとnumに記録されている数字がデクリメントされる

使い終わったら/mnt/pmem0/dataと/mnt/pmem1/numは手動で削除する必要がある