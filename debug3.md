# 是不是发送不成功啊

## bmbt
```txt
[   10.814052] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   10.816981] [huxueshi:mld_ifc_start_timer:1014] 33a
[   10.827226] [huxueshi:ip6_output:150]
[   10.827924] [huxueshi:ip6_finish_output:131]
[   10.828238] [huxueshi:ip6_finish_output2:115]
[   10.829208] [huxueshi:dst_neigh_output:462]
[   10.830208] [huxueshi:dev_queue_xmit:3249]
[   10.831203] [huxueshi:__dev_queue_xmit:3182]
[   10.831970] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   10.832206] [huxueshi:__dev_xmit_skb:2962]
[   10.833203] [huxueshi:sch_direct_xmit:158]
[   10.834343] [huxueshi:sch_direct_xmit:168] !!!!!!!!!!!!!!!
[   11.648078] [huxueshi:mld_ifc_timer_expire:2432]
[   11.649071] [huxueshi:mld_sendpack:1616]
[   11.649850] [huxueshi:ip6_output:150]
[   11.650089] [huxueshi:ip6_finish_output:131]
[   11.651407] [huxueshi:ip6_finish_output2:115]
[   11.652196] [huxueshi:dst_neigh_output:459]
[   11.653548] [huxueshi:dev_queue_xmit:3249]
[   11.654073] [huxueshi:__dev_queue_xmit:3182]
[   11.655069] [huxueshi:__dev_xmit_skb:2947] 54 0 1
[   11.656071] [huxueshi:__dev_xmit_skb:2974] !!!!!!!!!!!!!!!
[   11.829865] [huxueshi:mld_sendpack:1616]
[   11.830116] [huxueshi:ip6_output:150]
[   11.831080] [huxueshi:ip6_finish_output:131]
[   11.832067] [huxueshi:ip6_finish_output2:115]
[   11.833055] [huxueshi:dst_neigh_output:459]
[   11.833812] [huxueshi:dev_queue_xmit:3249]
[   11.834768] [huxueshi:__dev_queue_xmit:3182]
[   11.835057] [huxueshi:__dev_xmit_skb:2947] 54 0 2
[   11.836055] [huxueshi:__dev_xmit_skb:2974] !!!!!!!!!!!!!!!
[   11.840461] [huxueshi:ip6_output:150]
[   11.841074] [huxueshi:ip6_finish_output:131]
[   11.842572] [huxueshi:ip6_finish_output2:115]
[   11.843057] [huxueshi:dst_neigh_output:462]
[   11.844064] [huxueshi:dev_queue_xmit:3249]
[   11.845051] [huxueshi:__dev_queue_xmit:3182]
[   11.846055] [huxueshi:__dev_xmit_skb:2947] 54 0 3
[   11.847051] [huxueshi:__dev_xmit_skb:2974] !!!!!!!!!!!!!!!
[   12.150224] [huxueshi:mld_sendpack:1616]
[   12.151059] [huxueshi:ip6_output:150]
[   12.151740] [huxueshi:ip6_finish_output:131]
[   12.152001] [huxueshi:ip6_finish_output2:115]
[   12.152990] [huxueshi:dst_neigh_output:459]
[   12.153989] [huxueshi:dev_queue_xmit:3249]
[   12.154711] [huxueshi:__dev_queue_xmit:3182]
[   12.155001] [huxueshi:__dev_xmit_skb:2947] 54 0 4
[   12.155993] [huxueshi:__dev_xmit_skb:2974] !!!!!!!!!!!!!!!
```
## lat
```txt
[   10.703399] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
[   10.717872] [huxueshi:mld_ifc_start_timer:1014] 0
[   10.720169] [huxueshi:mld_ifc_timer_expire:2432]
[   10.722823] [huxueshi:mld_sendpack:1616]
[   10.730675] [huxueshi:ip6_output:150]
[   10.732547] [huxueshi:ip6_finish_output:131]
[   10.735955] [huxueshi:ip6_finish_output2:115]
[   10.736376] [huxueshi:dst_neigh_output:462]
[   10.737315] [huxueshi:dev_queue_xmit:3249]
[   10.737678] [huxueshi:__dev_queue_xmit:3182]
[   10.737872] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   10.738312] [huxueshi:__dev_xmit_skb:2962]
[   10.738662] [huxueshi:sch_direct_xmit:158]
[   10.740256] [huxueshi:dev_hard_start_xmit:2776]
[   10.740507] [huxueshi:xmit_one:2753]
[   10.740784] [huxueshi:netdev_start_xmit:3752]
[   10.742466] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   10.743517] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   10.746527] [huxueshi:mld_ifc_start_timer:1014] 150
[   11.083379] [huxueshi:ip6_output:150]
[   11.083570] [huxueshi:ip6_finish_output:131]
[   11.084097] [huxueshi:ip6_finish_output2:115]
[   11.084291] [huxueshi:dst_neigh_output:462]
[   11.084472] [huxueshi:dev_queue_xmit:3249]
[   11.084636] [huxueshi:__dev_queue_xmit:3182]
[   11.084812] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   11.084992] [huxueshi:__dev_xmit_skb:2962]
[   11.085051] [huxueshi:sch_direct_xmit:158]
[   11.085470] [huxueshi:dev_hard_start_xmit:2776]
[   11.085652] [huxueshi:xmit_one:2753]
[   11.085806] [huxueshi:netdev_start_xmit:3752]
[   11.085984] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   11.086352] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   11.086940] [huxueshi:mld_ifc_timer_expire:2432]
[   11.087292] [huxueshi:mld_sendpack:1616]
[   11.087500] [huxueshi:ip6_output:150]
[   11.087652] [huxueshi:ip6_finish_output:131]
[   11.087857] [huxueshi:ip6_finish_output2:115]
[   11.088142] [huxueshi:dst_neigh_output:459]
[   11.088525] [huxueshi:dev_queue_xmit:3249]
[   11.088708] [huxueshi:__dev_queue_xmit:3182]
[   11.088880] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   11.089063] [huxueshi:__dev_xmit_skb:2962]
[   11.089323] [huxueshi:sch_direct_xmit:158]
[   11.089490] [huxueshi:dev_hard_start_xmit:2776]
[   11.089667] [huxueshi:xmit_one:2753]
[   11.089809] [huxueshi:netdev_start_xmit:3752]
[   11.089987] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   11.090339] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   12.085201] [huxueshi:mld_sendpack:1616]
[   12.085456] [huxueshi:ip6_output:150]
[   12.085614] [huxueshi:ip6_finish_output:131]
[   12.085789] [huxueshi:ip6_finish_output2:115]
[   12.085960] [huxueshi:dst_neigh_output:459]
[   12.086290] [huxueshi:dev_queue_xmit:3249]
[   12.086470] [huxueshi:__dev_queue_xmit:3182]
[   12.086642] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   12.086825] [huxueshi:__dev_xmit_skb:2962]
[   12.086985] [huxueshi:sch_direct_xmit:158]
[   12.087263] [huxueshi:dev_hard_start_xmit:2776]
[   12.087456] [huxueshi:xmit_one:2753]
[   12.087607] [huxueshi:netdev_start_xmit:3752]
[   12.087795] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   12.088037] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   12.089598] [huxueshi:ip6_output:150]
[   12.089781] [huxueshi:ip6_finish_output:131]
[   12.090032] [huxueshi:ip6_finish_output2:115]
[   12.090301] [huxueshi:dst_neigh_output:462]
[   12.090479] [huxueshi:dev_queue_xmit:3249]
[   12.090650] [huxueshi:__dev_queue_xmit:3182]
[   12.090825] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   12.091124] [huxueshi:__dev_xmit_skb:2962]
[   12.091301] [huxueshi:sch_direct_xmit:158]
[   12.091463] [huxueshi:dev_hard_start_xmit:2776]
[   12.091650] [huxueshi:xmit_one:2753]
[   12.091800] [huxueshi:netdev_start_xmit:3752]
[   12.091973] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   12.092302] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   12.543362] [huxueshi:mld_sendpack:1616]
[   12.543593] [huxueshi:ip6_output:150]
[   12.543750] [huxueshi:ip6_finish_output:131]
[   12.543920] [huxueshi:ip6_finish_output2:115]
[   12.544097] [huxueshi:dst_neigh_output:459]
[   12.544204] [huxueshi:dev_queue_xmit:3249]
[   12.544204] [huxueshi:__dev_queue_xmit:3182]
[   12.544204] [huxueshi:__dev_xmit_skb:2947] 54 0 0
[   12.544204] [huxueshi:__dev_xmit_skb:2962]
[   12.544204] [huxueshi:sch_direct_xmit:158]
[   12.544204] [huxueshi:dev_hard_start_xmit:2776]
[   12.544204] [huxueshi:xmit_one:2753]
[   12.544204] [huxueshi:netdev_start_xmit:3752]
[   12.544204] e1000e: [KERNEL:e1000_xmit_frame:5821] 1
[   12.544204] e1000e: [huxueshi:e1000_intr_msix_tx:1948]
[   12.709774] e1000e: [huxueshi:e1000_intr_msix_rx:1967]
udhcpc: broadcasting discover
```

## 检查到是因为没有调用  netdev_tx_completed_queue


## 检查是因为 netdev_completed_queue 的参数是 0 了

## 如果 centos-qemu 的进行 dma 操作，那不是很难受吗
