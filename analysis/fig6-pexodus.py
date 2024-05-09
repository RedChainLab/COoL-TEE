#!/usr/bin/env python
# coding: utf-8

# In[ ]:


OUTPUT_DIR="G:/data"
FIGS_DIR="G:/figs"


# In[ ]:


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

pd.options.mode.chained_assignment = None  # default='warn'

from matplotlib.patches import Patch

import sys
import re


# In[ ]:


params = {'text.usetex' : True,
          'font.size' : 10,
            'font.family' : 'serif',
            'font.serif' : 'Computer Modern Roman',
          }
plt.rcParams.update(params) 


# %%
fig, ax=plt.subplots(figsize=(3, 2.5))
ax.plot([0,1],[1,1], label="$p^{timing,c_{M}=0}_{exodus}$", color="tab:blue", linestyle="dotted")
ax.plot([0,1],[1,.5], label="$p^{timing,c_{M}=0.5}_{exodus}$", color="tab:blue", linestyle="dashed")
ax.plot([0,1],[1,0], label="$p^{cuckoo-T}_{exodus}$", color="tab:blue", linestyle="solid")

#ax.plot(0.75,0.625, marker="+", markersize=10, markeredgecolor="black") 
ax.arrow(0.75,0.625,-0.25,0.125, width=0.01, length_includes_head=True,head_width=0.06,facecolor="black",zorder=10) 

#ax.plot(0.75,0.25, marker="+", markersize=10, markeredgecolor="black") 
ax.arrow(0.75,0.25,-0.25,0.25, width=0.01, length_includes_head=True,head_width=0.06,facecolor="black",zorder=10)

ax.text(0.5,0.875,"Fig.6b", ha="center", va="bottom", fontsize="small")
ax.text(0.75,0.75,"Fig.6a", ha="center", va="bottom", fontsize="small")
ax.vlines(.5,0,0.875, linestyle="solid", linewidth=1, color="black")
ax.vlines(.75,0,0.75, linestyle="solid", linewidth=1, color="black")

ax.plot([1,0],[0.5,.5], label="$p^{cuckoo-C,sat}_{exodus}$", color="tab:red", linestyle="solid")
ax.plot([1,0],[0,0], label="$p^{cuckoo-C,start}_{exodus}$", color="tab:red", linestyle="dashed")
ax.set_xticks(np.arange(0,1.01,0.25), labels=[f"{i}\%" for i in np.arange(0,101,25)])
ax.set_xticks(np.arange(0,1.01,0.05), minor=True)
ax.set_xlabel("Total system load $\\rho$")
ax.set_yticks(np.arange(0,1.01,0.25), labels=[f"{i}\%" for i in np.arange(0,101,25)])
ax.set_yticks(np.arange(0,1.01,0.05), minor=True)
ax.set_ylabel("Attack threshold $p_{exodus}^{<type>}$")
ax.grid(which="major",axis="both", alpha=1)
ax.grid(which="minor",axis="both", alpha=0.3)

# Shrink current axis by 20%
box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width, box.height*0.8])

# Put a legend to the right of the current axis
ax.legend(loc='center left', bbox_to_anchor=(-.25, 1.25),handlelength=1.5, ncols=2, labelspacing=0.25)
filename=f"{FIGS_DIR}/pexodus.pdf"
fig.savefig(filename, transparent=True, dpi=1000, bbox_inches='tight')
print(f"Saved {filename}")
# %%
