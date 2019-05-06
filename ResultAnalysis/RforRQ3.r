library(ggplot2)
library(reshape2)
library(cowplot)

figWidth<-25
figHeight<-4
textFont<-10
Basedir="D:\\Study\\Research Project\\ISSTA19\\RQ3\\"
#Basedir="/home/xia/DeepFL/Artifact/DeepFaultLocalization/ResultAnalysis/Rdata/"
Subs<-c("Overall")
losses<-c("epairwise","softmax")
model<-"dfl2"
#epochs=seq(from = 2, to = 60, by = 2)   # X axis
epochs=seq(from = 1, to = 2, by = 1)   # X axis
Metrics<-c("Top1","Top3","Top5","MFR","MAR")

for(sub in Subs){
	plot_list = list()
	count=1
	pdf(paste(c(Basedir,sub,".pdf"),collapse=""), width=figWidth, height=figHeight)

		for(me in Metrics){
				
			combinemet=cbind(epochs)
			for(loss in losses){
				datafile=paste(Basedir,loss,"/",sub,"_",model,".txt",sep="")
				datamodel<-read.table(datafile,sep = ",")
				colnames(datamodel) <- c("Top1","Top3","Top5","MFR","MAR")
				combinemet=cbind(combinemet,datamodel[,me])
			}
			combinemet=data.frame(combinemet)
			colnames(combinemet) <- c("epochs","pairwise","softmax")
			combinemet <- melt(combinemet, id.vars='epochs')
			colnames(combinemet) <- c("epochs","model","value")
			p<-ggplot(combinemet, aes(x=epochs, y=value, group=model, colour=model,shape=model)) +
				#geom_line()+geom_point(size=2)+ ylab(me)+scale_x_continuous(breaks=seq(5,61,by=5), limits=c(2,60))
				geom_line()+geom_point(size=2)+ ylab(me)+scale_x_continuous(breaks=seq(1,2,by=1), limits=c(0,2))
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
