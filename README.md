# pod_all

Core code for https://github.com/sec-bit/zkPoD-lib  

### ��ȡ����
1����libsnark����  
git submodule init && git submodule update

2����libsnark������  
cd depends/libsnark  
git submodule init && git submodule update

### ���루linux or osx��
1������libsnark��ʹ��mcl_bn128��Ҳ����ʹ��-DCURVE=BN128��-DCURVE=ALT_BN128������pod_core,pod_setup�Ķ���Ҳ��Ҫͬ���޸ģ�  
mkdir build  
cd build  

�����linux��g++����  
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=ON -DUSE_PT_COMPRESSION=OFF -DMONTGOMERY_OUTPUT=OFF -DBINARY_OUTPUT=OFF -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 -DUSE_ASM=ON ..  
�����osx��clang++��:  
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=OFF -DUSE_PT_COMPRESSION=OFF -DMONTGOMERY_OUTPUT=OFF -DBINARY_OUTPUT=OFF -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 -DUSE_ASM=OFF ..  

make  
make install  
�����dependsĿ¼�¶�һ��installĿ¼��  

2������pod_core��pod_setup��pod_publish��linux or osx��  
cd pod_core  
make  

cd ../pod_setup  
make  

cd ../pod_publish  
make  

����õĴ�����linux/binĿ¼�¡�  

3������pod_setup  
cd linux/bin  
./pod_setup  
�����������л���Ĭ��Ŀ¼����pk,vk��  

4������pod_publish����һ���ļ�  
cd linux/bin  
./pod_publish -m table -f test100000.csv -o table_data -t csv -k 0 1  

5������pod_core  
cd linux/bin  
./pod_core -m table -a atomic_swap_pod_vc -p table_data -o table_output --demand_ranges 1-10  


### ���� (windows + msvc2019)��  
1�����ȱ���libsnark���ο�libsnark/msvc/README.md��  
2��Ȼ��ֱ����msvc2019��pod_all.sln��  
