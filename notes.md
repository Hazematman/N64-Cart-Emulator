boot command for linux is 

```
boot_mmc=fatload mmc ${mmc_dev}:${mmc_boot_part} 45000000 ${kernel}; bootm 45000000
```

Boot address is `45000000`, can use this to test a baremetal program 

loadx 45000000; go 45000000
