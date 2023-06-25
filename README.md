# block design

[doc/system.pdf](doc/system.pdf)

# board
复位PC前，板子应停在打印 
```
Waiting for PCIe Link up
```
复位PC后，板子应停在打印 
```
PCIe Link up...
Bridge Init done...
```
加载驱动后，板子打印剩下全部

如果打印报告 Bar2 LO 是 0x0, 估计就不行了。

xpciepsu_ep_enable_example，没有配置中断，PIO测试会失败（卡死）。最新的板子代码可以通过PIO测试。


# driver 
zynqmp-pspcie-epdma
```
make
sudo make insert

sudo chmod 777 /dev/ps_pcie*
cd app/
./simple_test -c 0 -a 0x100000 -l 1024 -d s2c
./simple_test -c 1 -a 0x100000 -l 1024 -d c2s
./pio_test -o 0x0 -l 64
```
可查看PC物理地址分布
```
sudo cat /proc/iomem
```
查看dma的设备channel
```
sudo ls /sys/class/dma/ -l
```
查看PC中断，应该出现"PS PCIe DMA MSI Handler", 获取对应的中断号
```
sudo cat /proc/interrupts
```

# pcie_manage

xsct
```
mrd 0x100000
mwr -size b 0x100000 0xb5
```
那么可以在 pcie_manage 读到

反之，用 pcie_manage 写， 也可以在 xsct 里读到





