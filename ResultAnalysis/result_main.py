import result_analysis as result
from result_conf import *
import subprocess

def main():
	loss_function = 'softmax'
	tech = "DeepFL"
	first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
	
	if RQ_number == "RQ1":		
		dnns = ['dfl2']
		libsvm_models = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']		 
		model_name = ['Ochiai','Me-Ochiai','MULTRIC','FLUCCS','TraPT','MLP_DFL(2)']
		first_row = "Subjects,Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		result_matrix = result.read_libsvm(subs,models,libsvm_models)
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
		print("RQ1.csv DONE: you can find it in curret directory ./RQ1.csv")	
	if RQ_number == "RQ2":		
		dnns = ['mlp','mlp2','birnn','dfl1','dfl2']
		libsvm_models = ['TraptJhawkByteIR']
		model_name = ['LIBSVM','MLP','MLP2','BiRNN','MLP_DFL(1)','MLP_DFL(2)']
		models =  libsvm_models + dnns
		result_matrix = result.read_libsvm(subs,models,libsvm_models)
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
		print("RQ2.csv DONE: you can find it in curret directory ./RQ2.csv")	
	if RQ_number == "RQ2_2":		
		dnns = ['dfl1','dfl1-Spectrum','dfl1-Mutation','dfl1-Metrics','dfl1-Textual']
		m_names = ['MLP_DFL(1)','MLP_DFL(1)-SpectrumInfor','MLP_DFL(1)-MutationInfor','MLP_DFL(1)-MetricsInfor',
						'MLP_DFL(1)-TextualInfo']
		models = dnns	
		result_matrix = result.initialize_result(subs,dnns) 								
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,m_names,RQ_number,first_row,tech,result_matrix)	
		print("RQ2_2.csv DONE: you can find it in curret directory ./RQ2_2.csv")	
	
	if RQ_number == "RQ3":
		loss_funcs = ['softmax','epairwise']		
		dnns = ['dfl2']
		models = dnns
		model_name = ['MLP_DFL(2)']		
		new_epoch_number = 60		
		for e in range(2,int(new_epoch_number) + 1,2):
			for loss in loss_funcs:
				new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
				result_matrix = result.initialize_result(subs,dnns) 
				result.RQ(e,deep_data_dir,loss,models,new_subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
		subprocess.call('Rscript RforRQ3.r loss_eval',shell=True)
		print("RQ3.pdf DONE: you can find it in ./Rdata/RQ3.pdf")
  	
  	if RQ_number == "RQ4":
  		if "Cached" not in result_dir:
  			result.cross_vali_result()	
		techs = ['DeepFL','CrossDeepFL','CrossValidation']
		model_name = ['MLP_DFL(2)']   #not important
		dnns = ['dfl2']     
		models = dnns		
		new_epoch_number = 60		
		for e in range(2,int(new_epoch_number) + 1,2):
			for tech in techs:
				new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
				result_matrix = result.initialize_result(subs,dnns)
				result.RQ(e,deep_data_dir,loss_function,models,new_subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
  		subprocess.call('Rscript RforRQ4.r Cross',shell=True)
		print("RQ4.pdf DONE: you can find it in ./Rdata/RQ4.pdf")
		

#main function execution
if __name__=='__main__':
    main()