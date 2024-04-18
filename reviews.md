
SUBMISSION: 369
TITLE: COoL-TEE: Resilient Decentralized Search against Front-Running Attacks


----------------------- REVIEW 1 ---------------------
SUBMISSION: 369
TITLE: COoL-TEE: Resilient Decentralized Search against Front-Running Attacks
AUTHORS: Matthieu Bettinger, Sonia Ben Mokhtar, Anthony Simonet-Boulogne and Etienne Riviere

----------- Your overall rating -----------
SCORE: 1 (weak accept)
----------- Reviewer's expertise -----------
SCORE: 3 (knowledgeable)
----------- Paper strengths -----------
+ the paper identifies a vulnerability in DeSearch and proposes a solution
+ reasonable experimental evaluation
+ well-written paper
----------- Paper weaknesses -----------
- somehow incremental wrt DeSearch
- some aspects are not clear in the paper
----------- Detailed review -----------
The paper presents COoL-TEE, a technique that improves DeSearch, an earlier system. DeSearch handles the problem of decentralized marketplaces, in which some consumers may have an advantage with respect to other consumers if providers are malicious, and in special if such malicious providers collude with malicious consumers. DeSearch's solution is based on equipping providers with TEEs. A malicious provider may still delay a response to a consumer, creating the mentioned advantage. COoL-TEE addresses this problem with an improved provider selection, executed by consumers. Besides introducing DeSearch, the paper experimentally illustrates DeSearch's shortcoming, presents a solution, and experimentally evaluates it against DeSearch.

I very much enjoyed reading this paper. It addresses a timely problem, identifies what it seems to be a real vulnerability in an existing protocol and proposes a solution. The experimental evaluation is reasonably convincing (with a few caveats, discussed next).

On the negative side, the proposed idea is quite simple. Most of the complexity of the paper lies in DeSearch's elaborate design. COoL-TEE is a relatively simple extension. Additionally, I don't understand how COoL-TEE handles a simple attack in which malicious consumers send requests to all providers. This seems an obvious winning strategy.

There are a few other aspects that deserve further clarification. In the experimental evaluation, do providers receive assets instantaneously? Section VI.C, 2nd paragraph, you say that by artificially increasing latency, malicious providers can harm malicious consumers as well. This is obviously not true is malicious providers and malicious consumers collude.

The title of the paper is misleading in that the proposed technique does not address the problem of front-running attacks, but information head-starts. The paper even recognizes this in Section VII.B, 1st sentence.

A few typos:
- Section II.D.3, 3rd line" "correct responses"
- Section IV.C, paragraph before last: "theoretical"
- Section VI.C, 3rd paragraph, "As such"



----------------------- REVIEW 2 ---------------------
SUBMISSION: 369
TITLE: COoL-TEE: Resilient Decentralized Search against Front-Running Attacks
AUTHORS: Matthieu Bettinger, Sonia Ben Mokhtar, Anthony Simonet-Boulogne and Etienne Riviere

----------- Your overall rating -----------
SCORE: 1 (weak accept)
----------- Reviewer's expertise -----------
SCORE: 3 (knowledgeable)
----------- Paper strengths -----------
==*== Strengths
+ This paper offers COoL-TEE, a provider selection mechanism for decentralized search in decentralized marketplaces that is resilient to those information head-start attacks,
+ Solid motivations and reasonable threat models.
+ Extensive case studies.
----------- Paper weaknesses -----------
==*== Weaknesses
- Further in-depth comparisons with other similar works are needed.
- A brief introduction to the baseline solutions needs to be included.
- A series of important references appear to be missing.
----------- Detailed review -----------
==*==Paper Summary
Current decentralized marketplaces face issues with search functionality, lacking integration or relying on centralized search. Proposed decentralized search mechanisms, like DeSearch, are vulnerable to front-running attacks, granting malicious users an unfair advantage in asset discovery. This paper introduces COoL-TEE, a provider selection mechanism for decentralized search in marketplaces. COoL-TEE aims to counter information head-start attacks, ensuring resilience against such vulnerabilities, contingent on adequate honest computing power in the system.


Comments:
1. While this paper delivers a robust motivational statement and comprehensively reviews much of the classic literature, it is essential to note the absence of several key references. The following noteworthy works should be considered for inclusion:
[R1] While this article delivers a robust motivational statement and comprehensively reviews much of the classic literature, it is essential to note the absence of several key references. The following noteworthy works should be considered for inclusion:
[R2] Torres C F, Camino R. Frontrunner jones and the raiders of the dark forest: An empirical study of frontrunning on the ethereum blockchain[C]//30th USENIX Security Symposium (USENIX Security 21). 2021: 1343-1359.
[R3] Kelkar M, Deb S, Long S, et al. Themis: Fast, strong order-fairness in byzantine consensus[C]//Proceedings of the 2023 ACM SIGSAC Conference on Computer and Communications Security. 2023: 475-489.
[R4] Zhou L, Xiong X, Ernstberger J, et al. Sok: Decentralized finance (defi) attacks[C]//2023 IEEE Symposium on Security and Privacy (SP). IEEE, 2023: 2444-2461.

2. A brief introduction to the baseline solutions needs to be included. On one hand, the absence of a brief introduction to baseline solutions in this article creates certain obstacles for readers in comprehending the advantages presented. Additionally, the baseline solutions provided in this article appear to be relatively weak, potentially undermining the persuasiveness of the experimental results. It is recommended that the authors thoroughly discuss the baseline solutions and consider incorporating more advanced alternatives (e.g., [R5]) to enhance the overall robustness and credibility of the experimental outcomes.
[R5] M, Ishaq M, Magdon-Ismail M, et al. Fairmm: A fast and frontrunning-resistant crypto market-maker[C]//International Symposium on Cyber Security, Cryptology, and Machine Learning. Cham: Springer International Publishing, 2022: 428-446.

3. Further in-depth comparison with other similar works is needed. The authors conducted a comprehensive evaluation of the proposed solutions and baseline solutions, focusing on attack resistance and QoS. However, it appears that the selected baseline solutions may lack representativeness. For instance, a more straightforward approach involves applying traditional defenses to TEEs. Therefore, it is recommended to augment the baseline solutions by introducing additional defense mechanisms (e.g., [R6]) to further validate the superiority of COoL-TEE. This inclusion of diverse defenses will contribute to a more robust and nuanced assessment of the proposed solution.
[R6] Momeni P, Gorbunov S, Zhang B. Fairblock: Preventing blockchain front-running with minimal overheads[C]//International Conference on Security and Privacy in Communication Systems. Cham: Springer Nature Switzerland, 2022: 250-271.



----------------------- REVIEW 3 ---------------------
SUBMISSION: 369
TITLE: COoL-TEE: Resilient Decentralized Search against Front-Running Attacks
AUTHORS: Matthieu Bettinger, Sonia Ben Mokhtar, Anthony Simonet-Boulogne and Etienne Riviere

----------- Your overall rating -----------
SCORE: 1 (weak accept)
----------- Reviewer's expertise -----------
SCORE: 1 (unfamiliar)
----------- Paper strengths -----------
- relevant problem in modern distributed systems.
----------- Paper weaknesses -----------
- lack of context in the first part of the paper.
----------- Detailed review -----------
The paper addresses the problem of dealing with a kind of front-running attack affecting asset-searching algorithms in distributed market systems. The main contribution is Cool-TEE, an architecture to support and mediate the customer-provider-market interaction. In particular, the paper deals with the implementation of the provider selection module to prevent collusions favouring some customers with new information.

The paper is generally well written even if some parts would need to be improved to make it more readable and accessible also to non-expert readers.

In particular, the Introduction lacks context and it is difficult to understand the challenges behind the problem. This happens also later when the solution is presented. Summarizing, the solution leverages a kind of "trust" level built on top of previous interactions to select a good provider. However, the paper does not provide enough details on how the proposed mechanism contains collusions and on the technical challenges behind it.

Finally, the experimental evaluation does not summarize properly with respect to the 3 questions raised at the beginning of section V.



----------------------- REVIEW 4 ---------------------
SUBMISSION: 369
TITLE: COoL-TEE: Resilient Decentralized Search against Front-Running Attacks
AUTHORS: Matthieu Bettinger, Sonia Ben Mokhtar, Anthony Simonet-Boulogne and Etienne Riviere

----------- Your overall rating -----------
SCORE: -2 (reject)
----------- Reviewer's expertise -----------
SCORE: 4 (expert)
----------- Paper strengths -----------
COoL-TEE provides a trustless but reliable search mechanism for decentralized marketplaces.

It relies on two mechanisms: Trusted Execution Environments (TEEs) and Client-side Optimization of Latencies (COoL).

The paper does a good job motivating the problem, and it is fairly easy to follow.
----------- Paper weaknesses -----------
* All results are based on a simulation
* The paper lacks a good baseline that shows that COoL is the right approach
* The paper does not describe certain parts of the protocol well (see comments below)
----------- Detailed review -----------
Thank you for your submission. Unfortunately, I do not think the paper meets the threshold for publication because for multiple reasons.
That is not to say your approach is bad in any way. In contrary, if you address some of my comments, I think this would result in a great piece of work.

* All results are based on a simulation
        - Section 5B is very short and does not explain the assumptions your simulation makes
        - For example, what is the processing of a search provider and how long does it take to perform remote attestation?
        
* I feel like the paper is under-selling the proposed mechanism
        - There could be other useful applications for COoL outside of decentralized search mechanisms, e.g., anything that requires Oracles

* The paper lacks a good baseline that shows that COoL is the right approach
        - Obviously, comparing to a random selection you will perform better
        - Is there *any* other mechanism you can think of that you can compare COoL to?
        - This would help me understand some of the decisions you made when designing the algorithm better        

* How do we know the response from a provider is not stale?
        - In Figure 4, an attacker can delay/drop <NEW: A>. How would we detect that?
        - Is there some kind of verification with the blockchain itself? The paper does not discuss this.
        
* In figure 6, what is the total number of providers?
        - You scale from 1 to 8 malicious providers, but out of how many?

* The list of citation is pretty short and there are a few things the authors should discuss.
        - For example, as mentioned before, TEE-based Oracles work somewhat similar in that you query multiple of them for fault-tolerance and faster response times