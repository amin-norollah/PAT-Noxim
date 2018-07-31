PAT-Noxim - the NoC Simulator
=============================

Welcome to the PAT-Noxim, cycle accurate Network-on-Chip(NoC) simulator.


Description
------------
Networks-on-Chip (NoCs) have proven to be a low-latency and highly scalable in many-core architectures.
Due to the importance of scalability, designers try to optimize latency, power and temperature across the network.
Therefore, developing a precise tool to calculate the aforementioned attributes is of utmost importance. Designers
need to evaluate their proposed techniques in a NoC simulated environment. So, we propose PAT-Noxim to address the
shortcomings in design and post-design stages. PAT-Noxim, developed based on Access-Noxim, provides an environment
to simulate a NoC in power consumption, area, delay and temperature models. PAT-Noxim is developed to support
several predefined and custom architectures.


Structure
------------
PAT-Noxim works with three different simulators in harmony to cover aforementioned models:
1.	Orion 3.0 measures power consumption and area of routers by using orion model. We modified this simulator to work with PAT-Noxim.
2.	McPAT calculates area and power consumption of different processing elements, implemented by designer.
3.	Hotspot 6.0 receives area of routers and PEs from PAT-Noxim to calculate temperature more accurately than Access-Noxim.


What's New?
------------
Change list for the latest version of PAT-Noxim:
1.	Adding virtual channels 
2.	Implementing several router architectures
3.	Ability to change pipeline architectures such as 3-stage, 4-stage and 5-stage pipelines in runtime
4.	Adding credit signals to each router
5.	Improved power model for routers through Orion 3.0
6.	Improved area model for routers through Orion 3.0 to measure temperature more accurately in Hotspot 6.0
7.	Adding power and area models of several practical processors through McPAT
8.	Updating power and area of NoC with each change in router architecture.
9.	Adding support for 22, 32, 45, 65, 90 nm manufacturing technology.
10.	Calculating leakage current by Temperature Effect Inversion (TEI), taking into account the initial leakage current
11.	Measuring leakage current by TEI Through improved Orion 3.0
12.	Obtaining temperature feedback from tiles to calculate leakage current by TEI in a specific time interval.
13.	Increasing the accuracy of power and thermal measurements in sub-90 nm manufacturing technologies.
14.	Bug fixes 
