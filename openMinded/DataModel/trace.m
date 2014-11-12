run FdataFiltered.m
figure
plot(dataFiltered(:,1),dataFiltered(:,2),'r')
hold on
plot(dataFiltered(:,1),dataFiltered(:,3),'b')
plot(dataFiltered(:,1),dataFiltered(:,4),'y')
plot(dataFiltered(:,1),dataFiltered(:,5),'g')
plot(dataFiltered(:,1),dataFiltered(:,6),'b')
plot(dataFiltered(:,1),dataFiltered(:,7),'c')
grid on