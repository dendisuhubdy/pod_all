# pod_all

1����libsnark����  
git submodule init && git submodule update

2����libsnark������  
cd depends/libsnark  
git submodule init && git submodule update

3������libsnark  
mkdir build  
cd build  
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=ON -DWITH_PROCPS=OFF ..  
make  
make install  
�����dependsĿ¼�¶�һ��installĿ¼��

4������pod_core��pod_setup  
cd pod_core  
make  

5������pod_setup  
ע��pod_setup�������Ѿ����ı��ˡ�  
��ǰ��pod_setup�ǲ���һ��u���ⲿ�ִ����Ѿ�������pod_core��  
pod_core���Զ���û��ecc_pub.bin��ʱ�򴴽���  
pod_setup������zk trust setup��  
�����������л���Ĭ��Ŀ¼����pk,vk��  

6������pod_core
