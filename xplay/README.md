音视频同步
    1. 视频同步音频
    2. 音频同步视频
    3. 参考时钟同步


音频所需的精度更大，间隔更易被感知。所以音频同步更合适。

**线程退出**
生产消费模型一端停止另一端休眠等待时，并不能知道另一端退出而去醒来检查退出条件，需要主动唤醒去检查退出条件。


**sleep太多**

**锁太多，不够原子化**

**头一次实战中遇到死锁问题**

不是问AI还真找不出来，多线程bug出现很随机。

在XVideoThread和XAudioThread的run()方法中，有以下操作顺序：

首先获取XDecodeThread的mtx_锁（通过Pop()方法）
然后调用decode_->Send()或decode_->Recv()，这会获取XDecode的mtx_锁
而在Clear()方法中，操作顺序是相反的：

首先获取XDecodeThread的mtx_锁
然后调用decode_->Clear()，这需要获取XDecode的mtx_锁
这就可能造成死锁：

run()方法获取了XDecodeThread的mtx_锁，等待XDecode的mtx_锁
Clear()方法获取了XDecode的mtx_锁，等待XDecodeThread的mtx_锁
解决方法是修改Clear()方法，避免在持有XDecodeThread的mtx_锁时去获取XDecode的mtx_锁：