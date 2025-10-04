# GeorgiaTech-Campus-Navigation-Systems

**🗺️ Georgia Tech Campus Navigation System 🗺️**** ️佐治亚理工学院校园导航系统️**

Welcome to team 6's GitHub Organization! <br>欢迎来到team 6的GitHub组织！< br >
This is the home of our Campus Navigation System project — a fun (and hopefully useful) tool that helps students, staff, and visitors easily find their way around campus. 这是我们校园导航系统项目的主页-一个有趣的（希望有用的）工具，可以帮助学生，员工和访客轻松找到校园周围的路。

**What’s this project about?**这个项目是关于什么的？**

We aim to build a navigation system that makes campus routes easy to understand and provide clear and friendly route guidance. 我们的目标是建立一个导航系统，使校园路线易于理解，并提供清晰友好的路线引导。

Use cases: <br>   用例：
⏰ Students rushing to class <br>⏰学生们争先恐后地去上课
🏛️ Visitors exploring campus <br>🏛️参观校园的游客
⚡ Staff finding the fastest path between buildings <br>⚡工作人员找到建筑物之间的最快路径

<!--**What’s inside?**   < !…* *表面看来下一步要做什么呢?* *

This organization has multiple repositories, each focusing on a different piece of the puzzle:该组织有多个存储库，每个存储库都专注于拼图的不同部分：

graph-construction → where we turn the campus map into a graph图形构造→把校园地图变成图形

pathfinding-algorithms → all the shortest path magic happens here寻路算法→所有的最短路径魔法都发生在这里

frontend-ui → pretty maps and user-friendly directions前端ui→漂亮的地图和用户友好的方向

integration → putting everything together into a working system集成→把所有的东西组合成一个工作系统

**Our team**   * * * *我们的团队

We’re a group of students collaborating to bring this idea to life:我们是一群学生合作将这个想法变为现实：

Member A – Algorithm wizard成员A -算法向导

Member B – Data wrangler成员B -数据整理员
We construct a road–building adjacency graph for the Georgia Tech campus 我们为佐治亚理工学院校园构建了一个道路建设邻接图
using OpenStreetMap (OSM) data.使用OpenStreetMap （OSM）数据。

- **Roads**: only motor-vehicle-accessible roads are kept.  - **道路**：只保留机动车辆通行的道路。
- **Buildings**: each building is represented by its geometric centroid, which is projected to the nearest road segment (using point-to-segment distance).  - **建筑物**：每个建筑物由其几何质心表示，该质心投影到最近的路段（使用点对段距离）。
- **Projection**: the projection point is added to the graph, and the corresponding road edge is split to preserve distances.  - **投影**：将投影点添加到图形中，并拆分相应的道路边缘以保持距离。
- **Final Output**: an adjacency list **Adj**, where- **最终输出**：一个邻接表**Adj**，其中
  - For each node *i*, we store its set of neighbors that are directly connected.  -对于每个节点*i*，我们存储其直接连接的邻居集。
  - Each neighbor *j* is stored together with the corresponding edge distance *d(i,j)*.  —每个邻居*j*与对应的边距*d(i,j)*存储在一起。
  - Formally:   ——正式:
    ```
    Adj[i] = { (j, d(i,j)) | j ∈ Neighbors(i) }Adj[i] = {(j, d(i,j)) | j∈邻居(i)}
    ```
  
This pipeline transforms raw OSM data into a unified road–building graph model,该管道将原始OSM数据转换为统一的道路建设图模型，
which can be used for shortest-path search, parking analysis, and other 哪一个可以用于最短路径搜索、停车分析等
network-based computations.基于网络的计算。
Member C – UI/UX ninja成员C - UI/UX忍者

Member D – Integration guru会员D -集成专家

Member E – Documentation master (and README writer ✍️)-->会员E -文档管理员（和README作者✍️）- >

**Cool features**   * * * *冷却特性

🐾 Step-by-step directions with estimated travel time <br>🐾一步一步的方向，估计旅行时间
🛴 Easy-to-follow turn-by-turn instructions <br>🛴易于遵循的转弯指令
🚌 Future expansion: choose your transportation mode (walk, bike, shuttle!) <br>🚌未来扩展：选择你的交通方式（步行，自行车，穿梭！）< br >
