/* ============================================================================
 *  第 1 周 · 农田与仓库（顺序表）      templates/field.c
 *
 *  怎么用：
 *    1. 把本文件复制一份，放到 src/ 下面，名字必须是 field.c
 *       （VSCode 左侧：右键 src → 粘贴；或者另存为 src/field.c）
 *    2. 按任务书 T2 ~ T6 的顺序，把每个 TODO 换成你自己的代码
 *    3. Ctrl+Shift+B 编译，pf test 1 跑本周测试
 *
 *  说明：
 *    - 这个文件里 14 个函数一个都不能少。少一个，链接就会报
 *      undefined reference to 'xxx' —— 因为 src/field.c 一出现，
 *      src/stub.c 里第 1 周的临时实现就自动失效了。
 *    - 现在这些函数体是"空壳"：能编译，但什么也不做。
 *      所以刚复制过来时测试是红的，这是正常的。
 *    - 函数签名不要改，include/field.h 是怎么写的，你就得怎么写。
 * ========================================================================== */
#include "field.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* ============================================================================
 *  一、地图：6×6 的地块
 *
 *  g_farm.plots 是二维数组：plots[行][列]，也就是 plots[y][x]。
 *  6×6 的格子在同一块内存里挨着放，编译器帮你算 y*6+x。
 * ========================================================================== */

/* TODO T2 —— 取 (x,y) 这块地的指针；越界返回 NULL
 *
 *   下面这行"能跑"，但还差一件事：越界判断。
 *   （不判越界的话，输入 -1 或者 99 会读到数组外面去，程序直接崩。）
 *
 *   补成：
 *       if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return NULL;
 *       return &g_farm.plots[y][x];
 *
 *   顺便想清楚：为什么是 plots[y][x]，不是 plots[x][y]？
 */
Plot *plot_at(int x, int y)
{
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return NULL;
    return &g_farm.plots[y][x];
}

/* TODO T3 —— 开垦：荒地/枯萎地 -> 已开垦
 *
 *   条件：状态是 PLOT_WILD 或 PLOT_DEAD；体力 >= 10
 *   动作：state = PLOT_TILLED；crop_id = -1；water = 30；
 *         体力 -= 10；g_farm.till_count++；advance_time(600)
 *   返回：PF_OK / PF_PARAM（地块不对）/ PF_EMPTY（体力不够）
 */
int field_till(int x, int y)
{
    //判断越界
    if(x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return PF_PARAM;
    //获取地块指针
    Plot *p=&g_farm.plots[y][x];
    //判断地块状态和体力是否越界
    if(p->state!=PLOT_WILD && p->state!=PLOT_DEAD) return PF_PARAM;
    //判断体力
    if(g_farm.stamina<10) return PF_EMPTY;
    //判断地块状态
    p->state=PLOT_TILLED;
    //设置作物ID和水分
    p->crop_id=-1;
    p->water=30;
    //扣体力、增加开垦计数、推进时间
    g_farm.stamina-=10;
    g_farm.till_count++;
    advance_time(600);
    return PF_OK;
    // (void)x; (void)y;
    // return PF_PARAM;
}

/* TODO T3 —— 播种：已开垦 -> 已播种
 *
 *   条件：状态是 PLOT_TILLED；crop_by_id(crop_id) 找得到；金币够
 *   动作：扣种子钱；记 crop_id、planted_day、ripe_day = 今天 + grow_days
 *   返回：PF_OK / PF_PARAM / PF_EMPTY（钱不够）
 */
int field_sow(int x, int y, int crop_id)
{
    //判断越界
    if(x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return PF_PARAM;
    //获取地块指针
    Plot *p=&g_farm.plots[y][x];
    //判断地块状态
    if(p->state!=PLOT_TILLED) return PF_PARAM;
    //判断作物是否存在
    const Crop *crop=crop_by_id(crop_id);
    if(crop==NULL) return PF_PARAM;
    //判断金币够不够
    if(g_farm.gold <crop->seed_cost) return PF_EMPTY;
    //g_farm.gold是全局变量，表示玩家当前拥有的金币数（单位：分）。
    // crop->seed_cost是 Crop 结构体里的字段，表示这种作物的种子价格。
    //扣种子钱
    g_farm.gold -= crop->seed_cost;
    //记作物ID、种植日、成熟日
    p->crop_id = crop_id;
    p->state = PLOT_SEEDED;

    p->planted_day = g_farm.day;
    //g_farm.day是全局变量，表示当前的游戏天数（今天是游戏的第几天）。
    p->ripe_day = g_farm.day + crop->grow_days;
    //crop->grow_days用于获取指定作物的生长天数。
    //p->ripe_day表示该地块的作物成熟的日子
    return PF_OK;
}

/* TODO T3 —— 浇水：水分回满，荒地不能浇
 *
 *   返回：PF_OK / PF_PARAM
 */
int field_water(int x, int y)
{
    //判断越界
    if(x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return PF_PARAM;
    //获取地块指针
    Plot *p=&g_farm.plots[y][x];
    //判断地块状态
    if(p->state==PLOT_WILD) return PF_PARAM;
    //水分回满
    p->water=100;
    return PF_OK;
}

/* TODO T3 —— 收获：成熟 -> 产物进仓库，地块回到已开垦
 *
 *   产量 3~6 斤（3 + pf_rand(4））
 *   返回：实际斤数（>0）/ PF_PARAM（没熟）/ PF_FULL（仓库满）
 *   注意：仓库满了不能把地块改回已开垦，否则菜就白丢了
 */
int field_harvest(int x, int y)
{
    //判断越界
    if(x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return PF_PARAM;
    //获取地块指针
    Plot *p=&g_farm.plots[y][x];
    //判断地块状态
    if(p->state!=PLOT_RIPE) return PF_PARAM;
    //产量 3~6 斤（3 + pf_rand(4)）
    int yield = 3 + pf_rand(4);
    //pf_rand(4)是一个函数，用于生成一个0到3之间的随机整数。
    // 通过将其加上3，得到的产量范围为3到6斤。
    //调用 barn_add 函数将产物添加到仓库中 产物进仓库
    if(barn_add(&g_farm.barn, p->crop_id, yield) != PF_OK) return PF_FULL;
    //地块回到已开垦
    p->state = PLOT_TILLED;
    p->crop_id = -1;
    p->water = 30;
    return yield;
}

/* TODO T4 —— 换天：所有种着东西的地块长一天
 *
 *   对每个 SEEDED / GROWING / RIPE 的地块：
 *     water -= 25；water <= 0 时 40% 概率变 PLOT_DEAD
 *     到了 ripe_day -> PLOT_RIPE，否则 -> PLOT_GROWING
 */
void field_new_day(void)
{
    for(int y=0;y<WORLD_H;y++){
        for(int x=0;x<WORLD_W;x++){
            Plot *p=&g_farm.plots[y][x];
            if(p->state==PLOT_SEEDED || p->state==PLOT_GROWING || p->state==PLOT_RIPE){
                p->water-=25;//水分减少25
                if(p->water<=0){
                    //pf_rand(100) 会随机生成 0 到 99 之间的数字。
                    // 其中 0 到 39 刚好是 40 个数，概率就是 40%。
                    if(pf_rand(100)<40){
                        p->state=PLOT_DEAD;
                    }
                }else{
                    //判断是否成熟
                    if(g_farm.day>=p->ripe_day){
                        p->state=PLOT_RIPE;//成熟
                    }else{
                        p->state=PLOT_GROWING;
                    }
                }
            }
        }
    }
}

/* ============================================================================
 *  二、仓库：这是本周的主角，一个会自己长大的顺序表
 * ========================================================================== */

/* 初始化：向系统借 cap 个 Item 大小的内存
 *   返回：PF_OK / PF_PARAM / PF_NOMEM
 *   这段任务书里给了完整代码，可以直接抄（T3）
 */
int barn_init(Barn *b, int cap)//实现仓库的初始化
//barn为仓库，cap为容量
{
       if(!b || cap <= 0) return PF_PARAM;//返回一个叫 PF_PARAM 的错误码
       b->data =(Item *)malloc(sizeof(Item) * cap);
       //sizeof(Item) * cap 分配内存空间(每个Item的大小乘以容量 一共多少cap个Item)
       //将分配的内存空间的首地址赋值给b->data
       if(b->data==NULL){
        b->count = 0;
        b->cap = 0;
        return PF_NOMEM;//返回一个叫 PF_NOMEM 的错误码
       }
       b->count = 0;//初始化仓库中物品的数量为0
       b->cap = cap;//将仓库的容量设置为传入的参数cap
       return PF_OK;//返回一个叫 PF_OK 的成功码
}

/* 销毁：把借的内存还回去，指针置 NULL
 *   这段任务书里给了完整代码（T3）
 */
void barn_destroy(Barn *b)
{
    /* TODO T3：free + 置 NULL + count/cap 归零 */
     if(!b) return;
    free(b->data);//释放仓库中物品数据的内存空间
    b->data=NULL;//将仓库中物品数据的指针置为NULL，表示没有数据
    b->count=0;//将仓库中物品的数量归零
    b->cap=0;//将仓库的容量归零
    // (void)b;
}

/* TODO T4（本实验核心）—— 扩容：容量翻倍
 *
 *   1) int newcap = b->cap * 2;
 *   2) Item *p = realloc(b->data, newcap * sizeof(Item));
 *      —— 必须先用临时指针接住，不能直接赋给 b->data
 *   3) p == NULL -> return PF_NOMEM（原来的数据还在，不能丢）
 *   4) b->data = p; b->cap = newcap;
 *   5) cost_malloc(1);   // 记一次分配，引擎会折算成游戏时间
 */
int barn_grow(Barn *b)
{
    if(!b) return PF_PARAM;//返回一个叫 PF_PARAM 的错误码
    //cap<=0 说明这个仓库没初始化成功（或已经 destroy 过）。
    //这时候 newcap 也是 0，realloc(p, 0) 会返回一个"1 字节"的块而且不是 NULL，
    //下面就会误判成扩容成功、cap 还是 0，紧接着 barn_add 往 data[0] 写 —— 越界。
    //所以这里必须先挡住，不能靠 realloc 的返回值判断。
    if(b->cap<=0) return PF_PARAM;
    int newcap=b->cap*2;//将仓库的容量翻倍
    Item *p=(Item*)realloc(b->data,sizeof(Item)*newcap);
    //void *realloc(void *ptr, size_t new_size); 
    //  ptr：原来通过 malloc（或上一次 realloc）拿到的指针（旧地址）
    // new_size：希望变成多长的新总大小（单位是字节）。
    //使用realloc函数重新分配内存空间，将原来的数据拷贝到新的内存空间中

    if(p==NULL){
        //如果重新分配内存空间失败，返回一个叫 PF_NOMEM 的错误码
        // 并且不改变原来的数据
        return PF_NOMEM;
    }

        /*如果原本指针后面的空间足够大，它可能会直接在原地扩容，
                            这时候新指针和旧指针是一模一样的！*/
    // if(p!=b->data){
    //     //如果重新分配内存空间成功，并且新的内存空间的地址和原来的地址不一样
    b->data=p;//将仓库中物品数据的指针指向新的内存空间
    b->cap=newcap;//将仓库的容量设置为新的容量

        //搬东西需要花时间：记一次内存分配，引擎自己折算成游戏时间
    cost_malloc(1);
    
    // (void)b;
    // return PF_NOMEM;
    return PF_OK;//返回一个叫 PF_OK 的成功码
}
/* TODO T5 —— 入库：同一种作物合并数量，没有就追加
 *
 *   1) 先 barn_find 找有没有，有就 data[i].count += count
 *   2) 没有：count >= cap 就先 barn_grow
 *   3) 写到 data[count]，count++
 *   每一步比较都要调 cost_compare(1)
 */
int barn_add(Barn *b, int crop_id, int count)
{   if(!b || count <= 0) return PF_PARAM;//返回一个叫 PF_PARAM 的错误码  
    // 没初始化（cap<=0）就直接拒绝：否则会一路走到 barn_grow 再越界写
    if(b->cap<=0) return PF_PARAM;
       for(int i=0;i<b->count;i++){
        //遍历仓库中所有的物品，查找是否有相同的作物
        cost_compare(1);//调用cost_compare函数，记录一次比较操作
        if(b->data[i].crop_id==crop_id){
            //如果相同就返回1
            b->data[i].count+=count;
            //如果找到了相同的作物，就将该作物的数量加上传入的数量
            return PF_OK;//返回一个叫 PF_OK 的成功码
        }
    }//循环结束还没有返回值，说明没有找到相同的作物
          if(b->count>=b->cap){//如果仓库满了，才需要扩容（count 等于 cap 表示满了）
            int temp=barn_grow(b);//将仓库中物品的数量赋值给temp,调用扩容函数
            if(temp!=PF_OK){
                //如果扩容失败，返回一个叫 PF_NOMEM 的错误码
                return PF_NOMEM;
            }
        }//没有扩容失败就是成功扩容
            b->data[b->count].crop_id=crop_id;
            //没有找到相同的作物，将传入的作物添加到仓库中
            b->data[b->count].count=count;
            //没有找到相同的作物，就将传入的作物的数量设置为传入的数量
            //quality 是 1~5 星。新入库的作物还没评级，给个默认 1 星
            //（跟任务书进阶部分 MySQL 建表的 quality TINYINT DEFAULT 1 一致）。
            //不写这一行的话，malloc 出来的这块是"别人用完留下的垃圾"，
            //barn_save_csv 会把这个垃圾值一起存进存档。
            b->data[b->count].quality=1;
            b->count++;
            //没有找到相同的作物，就将传入的作物添加到仓库中，并将仓库中物品的数量加1
            return PF_OK;//返回一个叫 PF_OK 的成功码 
     // (void)b; (void)crop_id; (void)count;
    // return PF_NOMEM;
}


/* TODO T5 —— 按下标删除：后面的元素全部往前挪一格
 *
 *   挪动时调 cost_move(1)，这就是 O(n) 的代价
 *   返回：PF_OK / PF_PARAM（下标越界）/ PF_EMPTY（仓库空）
 */
int barn_remove_at(Barn *b, int pos, Item *out)
{
    if(!b || !out) return PF_PARAM;//返回一个叫 PF_PARAM 的错误码
    if(b->count==0) return PF_EMPTY;//如果仓库为空，返回一个叫 PF_EMPTY 的错误码
    if(pos<0 || pos>=b->count) return PF_PARAM;//如果pos越界，返回一个叫 PF_PARAM 的错误码
    //1.先备份，把要删的存下来 
    *out=b->data[pos];//将仓库中指定位置的物品赋值给out
    //2.要删除的元素的后面的元素往前挪
    for(int i=pos;i<b->count-1;i++){
        //将仓库中指定位置的物品的后面的元素往前挪
        b->data[i]=b->data[i+1];
        // 把 out 带出去，后面元素从前往前补，每挪一次 cost_move(1) 
        cost_move(1);//调用cost_move函数，记录一次移动操作
    }
    b->count--;//将仓库中物品的数量减1
    return PF_OK;//返回一个叫 PF_OK 的成功码
    // (void)b; (void)pos; (void)out;
    // return PF_NOMEM;
}


/* TODO T5 —— 查找：找到返回下标，找不到返回 -1 */
int barn_find(const Barn *b, int crop_id)
{
    if(!b) return -1;//如果仓库为空，返回-1
    for(int i=0;i<b->count;i++){
        cost_compare(1);//调用cost_compare函数，记录一次比较操作
        if(b->data[i].crop_id==crop_id){
            return i;//如果找到了相同的作物，返回其下标
        }
    }
    return -1;//如果没有找到，返回-1
}

/* TODO T6 —— 文件读写：把仓库存成 CSV / 从 CSV 读回来
 *   格式：crop_id,count,quality   第一行是表头
 *   任务书 §5 给了参考写法，可以抄
 */
int barn_load_csv(Barn *b, const char *path)
{   
    if(!b || !path) return PF_PARAM;//返回一个叫 PF_PARAM 的错误码
    FILE *fp=fopen(path,"r");//打开指定路径的CSV文件，读取仓库数据
    if(fp==NULL) return PF_PARAM;//如果文件打开失败，返回一个叫 PF_PARAM 的错误码
    char line[128];//定义一个字符数组，用于存储每一行的数据
    //读取第一行表头
    if(fgets(line,sizeof(line),fp)==NULL){
        fclose(fp);//关闭文件
        return PF_PARAM;//如果读取失败，返回一个叫 PF_PARAM 的错误码
    }
    //读取每一行数据
    while(fgets(line,sizeof(line),fp)!=NULL){
        int crop_id,count,quality;
        // 用 sscanf 从字符串里提取逗号隔开的三个整数
        if(sscanf(line,"%d,%d,%d",&crop_id,&count,&quality)==3){
         // 记录添加前的数量
        int old_count = b->count; 
        //将读取到的数据添加到仓库中  barn_add 会合并数量；如果没有，会追加
       int temp=barn_add(b,crop_id,count);
       if(temp!=PF_OK){
        fclose(fp);//关闭文件
        return temp;//如果添加失败，返回错误码
    }
    // 如果添加后仓库中物品的数量增加了，说明是新添加的物品，需要设置质量
    if (b->count > old_count) {
        b->data[b->count - 1].quality = quality;}
        }
    }
    fclose(fp);//关闭文件   
    return PF_OK;//返回一个叫 PF_OK 的成功码
    // (void)b; (void)path;
    // return PF_PARAM;
}

int barn_save_csv(const Barn *b, const char *path)
{
    if(!b || !path) return PF_PARAM;//返回一个叫 PF_PARAM 的错误码
    FILE *fp=fopen(path,"w");//打开指定路径的CSV文件，写入仓库数据
    if(fp==NULL) return PF_PARAM;//如果文件打开失败，返回一个叫 PF_PARAM 的错误码
    //写入表头
    if (fprintf(fp,"crop_id,count,quality\n")==EOF){
        fclose(fp);//关闭文件
        return PF_PARAM;//如果写入失败，返回一个叫 PF_PARAM 的错误码
    }
    //写入每一行数据
    for(int i=0;i<b->count;i++){
        if (fprintf(fp,"%d,%d,%d\n",b->data[i].crop_id,b->data[i].count,b->data[i].quality)==EOF){
            fclose(fp);//关闭文件
            return PF_PARAM;//如果写入失败，返回一个叫 PF_PARAM 的错误码
        }
    }
    fclose(fp);//关闭文件   
    return PF_OK;//返回一个叫 PF_OK 的成功码
    // (void)b; (void)path;
    // return PF_PARAM;
}
