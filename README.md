# FilesMergeSort

## 项目要求

题目：有序数据块排序

内容：定义“数据块”为有序数组，若干个数据块储存在单个 SSD 上，总大小 1TB，请给出对这若干个数据块的全局排序算法，要求内存使用不超过 16G，并在多核环境下的优化。

## 设计思路

- 针对多个有序数据块进行排列的问题，采用归并排序的方法来解决，为了提升效率，采用多路归并。
- 对于文件数据量比较大的情况，需要严格控制内存使用量，防止出现内存溢出。
  1. 首先，需要将内存中排好序的数据及时落盘，写出到文件中，借助磁盘文件辅助多路归并。
  2. 其次，在数据读取方面，也需要对每个文件每次读入内存中的数据块的大小做限制。
- 为了充分利用多核处理器的计算能力，对多路归并过程可以做并行优化，由多线程并行对若干文件数据进行排序。

## 实现方案

完整处理流程由两个模块组成，分别是文件处理模块和并行调度模块，在代码中分别由类`FileHandler`和类`Coordinator`表示。下面依次介绍这两个模块。

### FileHandler

文件处理模块主要负责对数据文件的字节流读取，能够确保单个数据文件加载到内存中的数据大小不超过类的成员变量`chunk_size_`。由于每条数据的长度大小不固定，在保证每次加载到内存中的数据块大小固定时，就有可能存在单条数据被切分，读取不完整的情况。所以该类需要在读到下一个 chunk 时，还原出完整的数据。

这些操作对外部是无感知的，该类的外部接口主要就是`NextRecord()`函数，相当于一个迭代器函数，外部只需要循环调用这个接口，就能将该文件中的数据正确、完整地读取。

### Coordinator

并行调度类负责全局的流程控制、任务分配、线程调度和同步工作，包含了文件多路归并排序的逻辑。原始输入数据文件和中间结果文件被划分为不同层级，原始输入数据文件为 Level 0，经过一次多路归并后生成的数据文件为 Level 1 等，直到输出最后一个结果文件。

调度类会根据系统内存限制、多路归并路数和数据块大小，计算出适当的并行线程数，来启动多个线程去执行分配的任务。每个线程会调用`FileHandler`类的接口，采用多路归并算法对数据进行排序，生成的结果文件放入下一个 Level 文件列表中。当线程完成当前任务后，会自动去请求下一个任务，如果全局任务所需线程数小于当前活跃线程数，那么该线程会自动退出。

## 使用方法

### 环境要求

- Cmake (Version >= 3.0)
- Make
- Googletest

### 编译

```shell
mkdir build && cd build
cmake ..
make
```

编译会生成三个可执行文件，将会存放在项目根路径的 bin 目录下：

- data_gen 自动生成测试数据文件，具有多个命令行参数可配置
- unit_tests 单元测试
- main 多文件并行排序算法可执行文件，指定输入数据存放目录和输出文件名

### 执行

#### 第一步

运行 data_gen 程序自动生成测试数据

```shell
/**
   Command line parameters:
     -n  number of files to generate
     -d  directory to output the generated files
     -l  average length of the values
     -v  variation of the values' lengths
     -f  file size (MB)
*/
./bin/data_gen -n 20 -d test/data_files/ -l 5 -v 3 -f 5
```

以上命令会在 test/data_files/ 路径下随机生成 20 个数据平均长度为 5，长度变化在 -3～+3 随机选择，文件大小为 5MB 的有序数据文件。

*Example：*

```
002Lujn
002O7Q
002Pa
002T5
002WS
002YzV
002doH
002iY
002iq
002lDn
002o
002r
002xO
002xQ
002y1
002yX
002z5
```

#### 第二步

运行 unit_tests 单元测试，共包含三个单元测试，分别是`FileHandler.ReadRecord`文件处理、数据读取测试，`Coordinator.Initialize`调度类初始化处理流程测试和`System.Run`整个系统运行测试。

```shell
./bin/unit_tests

[==========] Running 3 tests from 3 test suites.
[----------] Global test environment set-up.
[----------] 1 test from Coordinator
[ RUN      ] Coordinator.Initialize
[       OK ] Coordinator.Initialize (0 ms)
[----------] 1 test from Coordinator (0 ms total)

[----------] 1 test from FileHandler
[ RUN      ] FileHandler.ReadRecord
[       OK ] FileHandler.ReadRecord (416 ms)
[----------] 1 test from FileHandler (416 ms total)

[----------] 1 test from System
[ RUN      ] System.Run
[       OK ] System.Run (25339 ms)
[----------] 1 test from System (25339 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 3 test suites ran. (25755 ms total)
[  PASSED  ] 3 tests.
```

#### 第三步

运行 main 文件，完成对若干数据库的并行排序。

```shell
/**
   Command line parameters:
     -i  input data files' location
*/
./bin/main -i test/data_files

-----Merge Sort Level 0-----
Thread #0x585d000:
    data_file_0
    data_file_1  ------->>  data_file_0_1
   data_file_10
   data_file_11

-----Merge Sort Level 0-----
Thread #0x58e0000:
   data_file_12
   data_file_13  ------->>  data_file_12_1
   data_file_14
   data_file_15

-----Merge Sort Level 0-----
Thread #0x5963000:
   data_file_16
   data_file_17  ------->>  data_file_16_1
   data_file_18
   data_file_19

-----Merge Sort Level 0-----
Thread #0x59e6000:
    data_file_2
    data_file_3  ------->>  data_file_2_1
    data_file_4
    data_file_5

-----Merge Sort Level 0-----
Thread #0x5a69000:
    data_file_6
    data_file_7  ------->>  data_file_6_1
    data_file_8
    data_file_9

-----Merge Sort Level 1-----
Thread #0x58e0000:
  data_file_0_1
  data_file_6_1  ------->>  data_file_0_1_2
 data_file_16_1
 data_file_12_1

-----Merge Sort Level 1-----
Thread #0x59e6000:
  data_file_2_1  ------->>  data_file_2_1_2

-----Merge Sort Level 2-----
Thread #0x58e0000:
data_file_2_1_2  ------->>  sorted_result
data_file_0_1_2
```

从日志打印内容，可以看到多线程对这些文件的归并排序过程。

## 进一步优化

1. 在本项目中，多路归并的路数、并行线程数、每次读取文件的数据块大小和内存使用量限制之间是一个相互影响的关系，在多文件归并排序的过程中，如何动态调整归并路数和并行线程数来使得整体性能最优是一个值得深入研究的问题。
2. 采用一些 profiling 的工具，监控整个过程中的 CPU 使用率、内存占用量、文件 IO，可以帮助我们发现哪些地方是系统的瓶颈，哪些地方有进一步优化的空间。