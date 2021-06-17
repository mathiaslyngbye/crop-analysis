clc; clear; close;

data = readtable("data-old-set.csv");
%filter data
%
% 



mdl_height = fitlm(data.robot_z, data.image_z);
mdl_tilt = fitlm(data.robot_z, data.image_tilt);
mdl_height
mdl_tilt
% Plot height model

f = figure();
f.Position = [100 100 1000 600];

subplot(2,1,1)
plot(mdl_height);
title('')
xlabel('robot z [mm]') 
ylabel('image x [px]') 
set(gca,'FontSize',14)
set(gca,'linewidth',2)
set(gca,'FontWeight','Bold')
set(findall(gca, 'Type', 'Line'),'LineWidth',2);

subplot(2,1,2)

% Plot tilt model

plot(mdl_tilt);
%title('Tilt model')
title('') 
xlabel('robot z [mm]')
ylabel('image angle [deg]')
set(gca,'FontSize',14)
set(gca,'linewidth',2)
set(gca,'FontWeight','Bold')
set(findall(gca, 'Type', 'Line'),'LineWidth',2);
