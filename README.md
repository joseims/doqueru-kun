# doqueru-kun :snail:

![Doquerinho, our little, half giant snail half, container friend!](https://github.com/joseims/doqueru-kun/blob/master/images/doquerinho.jpg)

# Proposta  
Baseado [neste tutorial](http://cesarvr.github.io/post/2018-05-22-create-containers/?fbclid=IwAR115qJ_sKet0uQM3fJ6u1ALe9JHpEOldX4lE-HWVF_Fm-P0ctf6P9DcHJM) iremos criar nossa própria aplicação para containerização de programas. As features que iremos implementar são essas:

## Features Totais
:snail: : Isolamento do sistema de arquivos  
:snail: : Isolamento de namespace de users  
:snail: : Isolamento de variáveis  
:snail: : Isolamento da árvore de processos e das interfaces de rede  
:snail: : Controle do uso de memória  
:snail: :snail: : Em % (hard ou soft)  
:snail: :snail: : Ilimitado  
:snail: : Controle do uso de CPU  
:snail: :snail: : Em % (hard ou soft)  
:snail: :snail: : Ilimitado
 
Dessas features as coisas que diferem do tutorial original são: 


## Nossas mudanças  

:snail: : Usar pivot_root  
:snail: : Controle do uso de memória  
:snail: :snail: : Em % (hard ou soft)  
:snail: :snail: : Ilimitado  
:snail: : Controle do uso de CPU  
:snail: :snail: : Em % (hard ou soft)  
:snail: :snail: : Ilimitado  


## EXTRAS  
:snail: : Criar uma página na wikipedia sobre containers  
:snail: : Comunicação  
:snail: : Permitir controle de disco  
:snail: : Limitar/bloquear systemcalls


## Usage

>dokeru [*OPTION*]...
>
>--shares *CPU_SHARES*  
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
>define valor de CPU_SHARES  
>--period *CPU_PERIOD*  
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
>define valor de CPU_PERIOD  
>--percent *CPU_PERCENT*  
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
>define valor de CPU_PERCENT como um número de 1 a 100  
>--cpus *CPU_CPUS*  
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
>define valor de CPU_CPUS  
>--memlimit *MEM_MAX*  
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
>define valor de MEM_MAX  


## Oops!
- Quando dá `unshare -m` seguido de um `exit` 


https://yarchive.net/comp/linux/pivot_root.html
https://lists.gnu.org/archive/html/guix-commits/2015-06/msg00047.html
https://news.ycombinator.com/item?id=11558364
