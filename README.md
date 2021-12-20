# eeprom in flash

#### 介绍
本代码实现了简易的flash模拟eeprom功能，只需实现底层的flash操作接口，就可以在顶层调用eeprom接口使用。适合存储eeprom数据量较少的项目使用

#### 原理
将提供给eeprom使用的flash空间分为两个分区，每个分区可包含多个擦除页，擦除时统一擦除。当前使用哪一个分区由开始的USED_MARK决定，实际数据和eeprom虚拟地址共同组成一个编程单元，写入时新数据依次往后写入，读取时以最后一个值为有效值。当一个分区写满时会复制数据到另一个分区。

#### 移植说明
1.  将eeprom_in_flash.c和eeprom_in_flash.h复制到工程
2.  实现EE_ErasePart，EE_ProgramWord，EE_ReadWord底层调用接口，可参考eeprom_port示例
3.  修改eeprom_in_flash.h中的宏定义，配置flash地址大小等相关参数
    - EEPROM_NUM_MAX：eeprom数据的最大数量，单位为16bit数据的个数。可选范围受最小分区大小的限制，要小于(EEPROM_PART_SIZE_min/4-1)
    - EEPROM_PART0_SIZE/EEPROM_PART1_SIZE：两个分区的大小，可以是不同大小，每个分区可包含多个擦除页，为最小擦除页的整数倍
    - EEPROM_START_ADDRESS：用于模拟eeprom的flash起始地址

#### 使用说明
```c
//参数：eeprom初始化值，NULL则无初始值
int EEPROM_Init(void *default_data);
int EEPROM_Format(void *default_data);

//参数Address：eeprom的地址，一个地址保存16bit数据，范围0 - (EEPROM_NUM_MAX-1)
//参数length：读写buf的长度，为16bit数据的个数
uint16_t EEPROM_Read(uint16_t Address);
int EEPROM_Write(uint16_t Address, uint16_t Data);
int EEPROM_Read_Buf(uint16_t Address, uint16_t *buf, uint16_t length);
int EEPROM_Write_Buf(uint16_t Address, uint16_t *buf, uint16_t length);

//参数addr：eeprom存储空间的地址，单位byte，与上面eeprom的参数呈2倍关系，地址空间不可重复，必须2字节对齐，范围0 - (EEPROM_NUM_MAX-1)*2
//参数length：读写buf的长度，单位字节长度，必须2字节对齐
int Config_Read_Buf(uint16_t addr, void *buf, uint16_t length);
int Config_Write_Buf(uint16_t addr, void *buf, uint16_t length);
```

#### 使用限制

1.  最小编程单元为32bit及以下
2.  至少有两个可擦除页供eeprom使用
3.  储存的eeprom最大数据量小于可用flash空间的四分之一
4.  擦除后的清空值为0xff

#### 特点

1.  资源占有率极低，适合小型单片机项目
2.  支持flash擦除编程 **磨损平衡** 
3.  支持 **任意时间安全掉电** ，可靠性高
4.  全部数据在内存缓冲，读写速率快
5.  更新程序不影响储存的内容，可 **增量更新** ，也可以选择不使用老数据
6.  支持日志打印接口，方便调试
7.  支持8位单片机
8.  不支持数据正确性校验

#### 注意事项
1.  调用eeprom读写接口要注意地址和长度范围，不要出现不同数据地址范围覆盖的情况
2.  flash地址编程接口EE_ProgramWord，若不能32位写入原子操作，则要先写低16位后写高16位