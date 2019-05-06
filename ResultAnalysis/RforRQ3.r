library(ggplot2)
library(reshape2)
library(cowplot)

figWidth<-25
figHeight<-4
textFont<-10

# Xia only needs to change here to make it work

Basedir="C:\\Users\\Administrator\\Desktop\\ISSTA\\RQ5CrossTrend\\"
#Subs<-c("Lang","Chart","Time","Math","Closure","Mockito","Overall")
Subs<-c("Overall")
#losses<-c("epairwise","epairwiseSoftmax","softmax")
#losses<-c("epairwise","softmax")
losses<-c("within-project","cross-project","CrossValidation")
#Models<-c("birnn","rnn","mlp2","mlp")
#model<-"rnn"
model<-"bestV_MLP"
epochs=seq(from = 2, to = 60, by = 2)   # X axis
Metrics<-c("Top1","Top3","Top5","MFR","MAR")

for(sub in Subs){
	plot_list = list()
	count=1
	pdf(paste(c(Basedir,sub,".pdf"),collapse=""), width=figWidth, height=figHeight)

		for(me in Metrics){
				
			combinemet=cbind(epochs)
			for(loss in losses){
				datafile=paste(Basedir,loss,"\\",sub,"\\",model,".txt",sep="")
				datamodel<-read.table(datafile,sep = ",")
				#lines<-seq(5,100,by=5)
				#datamodel<-datamodel[lines,]
				colnames(datamodel) <- c("Top1","Top3","Top5","MFR","MAR")
				combinemet=cbind(combinemet,datamodel[,me])
			}
			combinemet=data.frame(combinemet)
			#colnames(combinemet) <- c("epochs","pairwise","pairwise+softmax","softmax")
			#colnames(combinemet) <- c("epochs","pairwise","softmax")
			colnames(combinemet) <- c("epochs","within-project","cross-project","CrossValidation")
			combinemet <- melt(combinemet, id.vars='epochs')
			colnames(combinemet) <- c("epochs","model","value")
			p<-ggplot(combinemet, aes(x=epochs, y=value, group=model, colour=model,shape=model)) +
				geom_line()+geom_point(size=2)+ ylab(me)+scale_x_continuous(breaks=seq(5,61,by=5), limits=c(2,60))
			plot_list[[count]] = p
			count=count+1
				
		}
			
		prow<-plot_grid(plot_list[[1]]+theme(legend.position="none"),
						plot_list[[2]]+theme(legend.position="none"),
						plot_list[[3]]+theme(legend.position="none"),
						plot_list[[4]]+theme(legend.position="none"),
						plot_list[[5]]+theme(legend.position="none"),
						#nrow = 1, align = 'h',labels = sub)
						nrow = 1, align = 'h')
		legend_b <- get_legend(plot_list[[1]] + theme(legend.position=c(0.933,4.94)))
		p <- plot_grid( prow, legend_b, ncol = 1, rel_heights = c(1, .2))
		print(p)
		dev.off()
	
}
