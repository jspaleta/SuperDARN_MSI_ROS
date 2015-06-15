#! /usr/bin/env python
import sys
import json
import rtserver
import msg
from StringIO import StringIO


def process_data():
	
	#take in arguments and send to rtserver.c initialize method
	c = sys.argv[1:]
	sock = rtserver.initialize(0,c)
	
	#forever loop
	while(1):
		io = StringIO()
		
		#calls rtserver method that returns processed Prm data
		prm = rtserver.getRadarParm();
		
		#proceeds to seperate the loaded in data then load it into a dictionary
		p_data = {'revision':{'major':prm.revision.major,'minor':prm.revision.minor}}
		p_data['origin'] = {'code':prm.origin.code,'time':prm.origin.time,
									'command':prm.origin.command}
		p_data['cp'] = prm.cp
		p_data['stid'] = prm.stid
		p_data['time'] = {'yr':prm.time.yr,'mo':prm.time.mo,'dy':prm.time.dy,
									'hr':prm.time.hr,'mt':prm.time.mt,'sc':prm.time.sc,
									'us':prm.time.us}
		p_data['txpow'] = prm.txpow
		p_data['nave'] = prm.nave
		p_data['atten'] = prm.atten
		p_data['lagfr'] = prm.lagfr
		p_data['smsep'] = prm.smsep
		p_data['ercod'] = prm.ercod
		p_data['stat'] = {'agc':prm.stat.agc,'lopwr':prm.stat.agc}
		p_data['noise']= {'search':prm.noise.search,'mean':prm.noise.mean}
		p_data['channel'] = prm.channel
		p_data['bmnum'] = prm.bmnum
		p_data['bmazm'] = prm.bmazm
		p_data['scan'] = prm.scan
		p_data['rxrise'] = prm.rxrise
		p_data['intt'] = {'sc':prm.intt.sc,'us':prm.intt.us}
		p_data['txpl'] = prm.txpl
		p_data['mpinc'] = prm.mpinc
		p_data['mppul'] = prm.mppul
		p_data['mplgs'] = prm.mplgs
		p_data['mplgexs'] = prm.mplgexs
		p_data['nrang'] = prm.nrang
		p_data['frang'] = prm.frang
		p_data['rsep'] = prm.rsep
		p_data['xcf'] = prm.xcf
		p_data['tfreq'] = prm.tfreq
		p_data['offset'] = prm.offset
		p_data['ifmode'] = prm.ifmode
		p_data['mxpwr'] = prm.mxpwr
		p_data['lvmax'] = prm.lvmax
	
		#load in the data for for mppul and lag data
		ptab = []
		i = 0
		while(i < prm.mppul):
			ptab.append(rtserver.return_pulse(i))
			i = i+1
		p_data['ptab'] = ptab
		
		if prm.lag is None:
			exit(1)
		
		if prm.mplgexs != 0:
			size =prm.mplgexs
		else:
			size = prm.mplgs
		ltag = []
		ltag0 =[]
		ltag1=[]
		i = 0
		while(i<=size):
			ltag0.append(rtserver.return_lag(0,i))
			ltag1.append(rtserver.return_lag(1,i))
			i=i+1
		ltag.append(ltag0)
		ltag.append(ltag1)
		p_data['ltag'] = ltag
		p_data['comf'] = prm.combf
		
		#Load the prm dictionary into a json then load it into the
		#stringIO functionality
		pdata = json.dumps(p_data)
		json.dump([pdata],io)
	
		#Load in Fit data from rtserver's getFitData method
		fit = rtserver.getFitData();
		f_data = {'revision':{'major': fit.revision.major,'minor': fit.revision.minor}}
		noise = rtserver.return_noise()
		f_data['noise'] = {'vel':noise.vel,'skynoise':noise.skynoise,
			'lag0':noise.lag0}
		#all of the structures rng, xrng and elv are formed of arrays this is 
		#their declaration
		if fit.rng is not None:
			i = 0
			x_data = 0
			p_0=[]
			v = []
			v_err=[]
			p_l = []
			p_l_err = []
			p_s = []
			p_s_err = []
			w_l= []
			w_l_err = []
			w_s = []
			w_s_err = []
			sdev_l = []
			sdev_s = []
			sdev_phi = []
			qflg = []
			gsct = []
			nump = []
			slist = []
			xp_0=[]
			xv = []
			xv_err=[]
			xp_l = []
			xp_l_err = []
			xp_s = []
			xp_s_err = []
			xw_l= []
			xw_l_err = []
			xw_s = []
			xw_s_err = []
			xphi0 = []
			xphi0_err = []
			xsdev_l = []
			xsdev_s = []
			xsdev_phi = []
			xqflg = []
			xgsct = []
			normal =[]
			high = []
			low = []
			
			#while loop that loads in each of the array data from return_rng_xrng
			#only loads in data if rng.qflg ==1 or xrng is not None and xrng.qflg ==1
			while(i<prm.nrang):
				rng = rtserver.return_rng_xrng(i,0)
				xrng = rtserver.return_rng_xrng(i,1)
				p_0.append(rng.p_0)
				if((rng.qflg==1) or
					(xrng is not None and xrng.qflg == 1)):
					x_data = 1
					v.append(rng.v)
					v_err.append(rng.v_err)
					p_l.append(rng.p_l)
					p_l_err.append(rng.p_l_err)
					p_s.append(rng.p_s)
					p_s_err.append(rng.p_s_err)
					w_l.append(rng.w_l)
					w_l_err.append(rng.w_l_err)
					w_s.append(rng.w_s)
					w_s_err.append(rng.w_s_err)
					sdev_l.append(rng.sdev_l)
					sdev_s.append(rng.sdev_s)
					sdev_phi.append(rng.sdev_phi)
					qflg.append(rng.qflg)
					gsct.append(rng.gsct)
					nump.append(rng.nump)
					slist.append(i)
					if prm.xcf != 0:
						xv.append(xrng.v)
						xv_err.append(xrng.v_err)
						xp_l.append(xrng.p_l)
						xp_l_err.append(xrng.p_l_err)
						xp_s.append(xrng.p_s)
						xp_s_err.append(xrng.p_s_err)
						xw_l.append(xrng.w_l)
						xw_l_err.append(xrng.w_l_err)
						xw_s.append(xrng.w_s)
						xw_s_err.append(xrng.w_s_err)
						xphi0.append(xrng.phi0)
						xphi0_err.append(xrng.phi0_err)
						xsdev_l.append(xrng.sdev_l)
						xsdev_s.append(xrng.sdev_s)
						xsdev_phi.append(xrng.sdev_phi)
						xqflg.append(xrng.qflg)
						xgsct.append(xrng.gsct)
						
						elv = rtserver.return_elv(i)
						normal.append(elv.normal)
						high.append(elv.high)
						low.append(elv.low)
				i += 1
		
		if(x_data):
			f_data['rng'] = {'v': v,'v_e': v_err,'pwr0':p_0,'p_l': p_l, 
				'p_l_e':p_l_err,'p_s':p_s,'p_s_e':p_s_err,'w_l':w_l,
				'w_l_e':w_l_err,'w_s':w_s,'w_s_e':w_s_err,'sd_l':sdev_l,
				'sd_s':sdev_s,'sd_phi':sdev_phi,'qflg':qflg,'gflg':gsct,
				'nlag':nump,'slist':slist}
			
			f_data['xrng'] = {'x_v': xv,'x_v_e': xv_err,'x_p_l': xp_l,
				'x_p_l_e':xp_l_err,'x_p_s':xp_s,'x_p_s_e':xp_s_err,'x_w_l':xw_l,
				'x_w_l_e':xw_l_err,'x_w_s':xw_s,'x_w_s_e':xw_s_err,'x_phi0':xphi0,
				'x_phi0_e':xphi0_err,'x_sd_l':xsdev_l,'x_sd_s':xsdev_s,
				'x_sd_phi':xsdev_phi,'x_qflg':xqflg,'x_gflg':xgsct}
		
			f_data['elv'] = {'elv':normal,'elv_low':low,'elv_high':high}
		else:
			f_data['rng'] = {'pwr0':p_0}
	
		fdata = json.dumps(f_data)
		json.dump([fdata],io)
		
		#writes data to the rtserver outpipe location
		outpipe= rtserver.get_outpipe()
		msg.ConnexWriteIP(outpipe,io.getvalue(),sys.getsizeof(pdata)+sys.getsizeof(fdata))
		sock = rtserver.initialize(sock,c)
	
if __name__ == '__main__':
    process_data()
