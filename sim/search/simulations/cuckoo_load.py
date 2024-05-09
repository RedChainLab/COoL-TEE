from fractions import Fraction as F

def cuckoo_load(p_max_thr:int, rho_total:F, rho_hon:F, n_prov:int, r_hm:F, x1:F, print_vec=True, print_p_exodus=False):
    lr_hon=n_prov*rho_total*p_max_thr*r_hm
    lr_x1_hon=lr_hon*x1
    for f in range(1,n_prov+1)[::-1]:
        lr_x1_hon_hon=lr_x1_hon*F(n_prov-f,n_prov)
        lr_x1_hon_mal=lr_x1_hon*F(f,n_prov)
        #print(F(n_prov-f,n_prov),F(f,n_prov))
        lp_hon=rho_hon*p_max_thr*(n_prov-f)
        lr_mal=n_prov*rho_total*p_max_thr*(1-r_hm)
        if lp_hon-lr_x1_hon_hon > lr_mal:
            lr_mal_mal=0
            lr_mal_hon=lr_mal
            lr_x3_hon_hon=lp_hon-lr_mal-lr_x1_hon_hon
            lr_x3_hon_mal=lr_hon-lr_x1_hon-lr_x3_hon_hon
            if print_p_exodus:
                if lr_x3_hon_mal>0:
                    print(F(f,n_prov),"p_exodus_cuckoo-T")
                else:
                    print(F(f,n_prov),"OK")
        else:
            if print_p_exodus:
                print(F(f,n_prov),"p_exodus_timing")
            lr_mal_hon=lp_hon-lr_x1_hon_hon
            lr_mal_mal=lr_mal-lr_mal_hon
            lr_x3_hon_hon=0
            lr_x3_hon_mal=lr_hon-lr_x1_hon
        #print("lr_mal",lr_mal,"mal_hon",lr_mal_hon,"mal_mal", lr_mal_mal)
        #print("lr_hon",lr_hon,"hon_hon",lr_x1_hon_hon,lr_x3_hon_hon,"hon_mal",lr_x1_hon_mal,lr_x3_hon_mal)
        
        assert lr_mal==lr_mal_hon+lr_mal_mal
        assert lr_hon==lr_x1_hon_hon+lr_x1_hon_mal+lr_x3_hon_hon+lr_x3_hon_mal
        assert lr_mal+lr_hon==p_max_thr*n_prov*rho_total

        #print(f)
        #print([(F(lr_x1_hon_hon+lr_x3_hon_hon,(n_prov-f)*lr_hon),F(lr_mal_hon,(n_prov-f)*lr_mal)) for _ in range(n_prov-f)] +\
        #[(F(lr_x1_hon_mal+lr_x3_hon_mal,f*lr_hon),F(lr_mal_mal,f*lr_mal)) for _ in range(n_prov-f,n_prov)])
        values=[F(lr_mal_hon,(n_prov-f)*lr_mal) for _ in range(n_prov-f)]+[F(lr_mal_mal,f*lr_mal) for _ in range(n_prov-f,n_prov)]
        output=f"{'${Vector{val:' if f==n_prov else '   Vector{val:'}{[str(frac.numerator)+'/'+str(frac.denominator) if frac.numerator>0 else 0 for frac in values]}{'},|' if f>1 else '} ! sHM}'}"
        output=output.replace("|","\\").replace("'","")
        if print_vec:
            print(output)

if __name__ == "__main__":
    PRINT_VEC=False
    PRINT_P_EXODUS=True

    LR=960
    P_MAX_THR=160
    X1=F(1/10)
    R_HM=F(5/10)

    TARGET_TIMING_ATT_LOAD=F(95,100)

    START_N_PROV=10
    STOP_N_PROV=24
    STEP_N_PROV=2

    for n_prov in range(START_N_PROV,STOP_N_PROV+1,STEP_N_PROV):
        print("n_prov:",n_prov,"rho:",F(LR,n_prov*P_MAX_THR))
        # Cuckoo-timing
        cuckoo_load(P_MAX_THR,F(LR,n_prov*P_MAX_THR),TARGET_TIMING_ATT_LOAD,n_prov,R_HM,X1,print_vec=PRINT_VEC, print_p_exodus=PRINT_P_EXODUS)
        # Cuckoo-content
        cuckoo_load(P_MAX_THR,F(LR,n_prov*P_MAX_THR),F(LR,n_prov*P_MAX_THR),n_prov,R_HM,X1,print_vec=PRINT_VEC, print_p_exodus=PRINT_P_EXODUS)