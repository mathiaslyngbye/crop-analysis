clc; clear; close;

data = readtable("data-old-set.csv")
%filter data
%
% 

mdl_height = fitlm(data.robot_z, data.image_z);
mdl_tilt = fitlm(data.robot_z, data.image_tilt);

% Plot height model
subplot(2,1,1);
plot(mdl_height);
title('Height model')
xlabel('robot z') 
ylabel('image x') 

% Plot tilt model
subplot(2,1,2);
plot(mdl_tilt);
title('Tilt model')
xlabel('robot z') 
ylabel('image tilt') 

mdl_height