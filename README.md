# pod_all

1����libsnark����  
git submodule init && git submodule update

2����libsnark������  
cd depends/libsnark  
git submodule init && git submodule update

3������libsnark��ʹ��mcl_bn128��Ҳ����ʹ��-DCURVE=BN128��-DCURVE=ALT_BN128������pod_core,pod_setup�Ķ���Ҳ��Ҫͬ���޸ģ�  
mkdir build  
cd build  
�����linux��
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=ON -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 ..  
�����osx:
cmake -DCMAKE_INSTALL_PREFIX=../../install -DMULTICORE=OFF -DWITH_PROCPS=OFF -DWITH_SUPERCOP=OFF -DCURVE=MCL_BN128 -DUSE_ASM=OFF ..  
make  
make install  
�����dependsĿ¼�¶�һ��installĿ¼��  

4������pod_core��pod_setup  
cd pod_core  
make  

5������pod_setup  
�����������л���Ĭ��Ŀ¼����pk,vk��  

6������pod_publish����һ���ļ�  
-m table -f test100000.csv -o table_data -t csv -k 0 1  

6������pod_core  
-m table -a atomic_swap_pod_vc -p table_data -o table_output --demand_ranges 1-10  