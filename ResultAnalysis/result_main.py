import result_analysis as result
from result_conf import *
import rpy2.robjects as robjects

def main():
	if RQ_number == "RQ1":
		loss_function = 'softmax'
		tech = "DeepFL"
		dnns = ['dfl2']
		libsvm_models = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']		 
		model_name = ['Ochiai','Me-Ochiai','MULTRIC','FLUCCS','TraPT','MLP_DFL(2)']
		first_row = "Subjects,Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row,tech)
	if RQ_number == "RQ2":
		loss_function = 'softmax'
		tech = "DeepFL"
		dnns = ['mlp','mlp2','birnn','dfl1','dfl2']
		libsvm_models = ['TraptJhawkByteIR']
		model_name = ['LIBSVM','MLP','MLP2','BiRNN','MLP_DFL(1)','MLP_DFL(2)']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row,tech)
	if RQ_number == "RQ2_2":
		loss_function = 'softmax'
		techs = ['DeepFL','DeepFL-Spectrum','DeepFL-Mutation','DeepFL-Metrics','DeepFL-Textual']
		m_names = ['MLP_DFL(1)','MLP_DFL(1)-SpectrumInfor','MLP_DFL(1)-MutationInfor','MLP_DFL(1)-MetricsInfor',
						'MLP_DFL(1)-TextualInfo']
		dnns = ['dfl1']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR" 
		libsvm_models = []
		models =  libsvm_models + dnns
		n = len(techs)
		for idx in range(n):
			tech = techs[idx]
			model_name = [m_names[idx]]
			new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']								
			result.RQ(epoch_number,deep_data_dir,loss_function,models,new_subs,dnns,libsvm_models,model_name,RQ_number,first_row,tech)		
	if RQ_number == "RQ3":
		loss_funcs = ['softmax','epairwise']
		tech = "DeepFL"
		dnns = ['dfl2']
		libsvm_models = []
		model_name = ['MLP_DFL(2)']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		for e in range(int(epoch_number)):
			e = e + 1
			for loss in loss_funcs:
				new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
				result.RQ(e,deep_data_dir,loss,models,new_subs,dnns,libsvm_models,model_name,RQ_number,first_row,tech)
		r_source = robjects.r['source']
  		r_source('RforRQ3.r') 
  	if RQ_number == "RQ4":
		loss_function = 'softmax'
		#techs = ['DeepFL','CrossDeepFL','CrossValidation']
		techs = ['DeepFL','CrossDeepFL']
		model_name = ['MLP_DFL(2)']   #not important
		dnns = ['dfl2']            
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"   #not important
		libsvm_models = []
		models =  libsvm_models + dnns
		for e in range(int(epoch_number)):
			e = e + 1
			for tech in techs:
				new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
				result.RQ(e,deep_data_dir,loss_function,models,new_subs,dnns,libsvm_models,model_name,RQ_number,first_row,tech)
  		
	

#main function execution
if __name__=='__main__':
    main()