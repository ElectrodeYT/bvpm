#!/bin/bash
rm busybox.bvp
(cd busybox-package; tar -cf busybox.bvp *)
mv busybox-package/busybox.bvp .
