# block

也许了解一下这个经典设备可以帮助我来理解 aio 之类的蛇皮到底在干什么

- [ ] 将 glib.md 中的测试分析首先放一下吧, 先分析 block/null.c

## https://www.linux-kvm.org/images/b/b5/2012-fourm-block-overview.pdf

- virtual device
  - IDE, virtio-blk
- Backend
  - block driver
    - raw, qcow2
    - file, nbd(networking block device), iscsi(nternet Small Computer Systems Interface), gluster(Gluster is a scalable network filesystem. Using common off-the-shelf hardware, you can create large, distributed storage solutions for media streaming, data ...)
  - I/O throttling, copy on read
- block jobs
  - Streaming, mirroring, commit,

```txt
aio=aio
  aio is "threads", or "native" and selects between pthread based disk I/O and native Linux AIO.
```

## block/null.c
- [ ] 如何使用?
bdrv_driver_preadv
