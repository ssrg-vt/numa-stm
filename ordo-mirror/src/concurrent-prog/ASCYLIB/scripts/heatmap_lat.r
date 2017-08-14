if (!require("RColorBrewer")) {
install.packages("RColorBrewer")
library(RColorBrewer)
}
library(reshape2)
require(gplots)
library(gplots)
require(RColorBrewer)
args <- commandArgs()
print(args[7])
ll <- read.csv(args[5],sep=",",check.names=FALSE)
row.names(ll) <- ll[,1]
nc <-ncol(ll)-1
ll <- ll[,2:nc]
ll_matrix <- data.matrix(ll)
col = brewer.pal(11,"RdYlGn")
myBreaks=c(0, 0.33, 0.50, 0.75, 0.85, 0.97, 1.03, 1.1, 1.3, 1.5, 2, 10000)
#myBreaks=c(10000, 2, 1.5, 1.3, 1.1, 1.03, 0.97, 0.85, 0.75, 0.50, 0.33, 0)
#now to save in pdf
pdf(file=args[6])
par(cex.main=0.75)
hm <- heatmap.2(1/ll_matrix, scale="none", Rowv=NA, Colv=NA,
                col =col, ## using your colors,
                cellnote=ll_matrix,
                notecol="black",
                notecex=0.75,
                breaks = myBreaks, ## using your breaks
                dendrogram = "none",  ## to suppress warnings
                margins=c(5,5), cexRow=1.0, cexCol=1.0, 
                ylab=args[9], xlab=args[8], main=args[7],
                key=FALSE, 
                lhei = c(1.5,10),
                lwid = c(1.5,10),
                trace="none")
dev.off()
