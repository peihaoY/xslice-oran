import gym
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.distributions import Normal
import matplotlib.pyplot as plt
import numpy as np
from env import slice_env
import time
import os
from gcn import DynamicGNN
import csv
import signal
import sys

user_num = 10    #

# Hyperparameters
learning_rate =  0.005 #0.005
gamma = 0.1
lmbda = 0.95
eps_clip = 0.9
K_epoch = 10
rollout_len = 3
buffer_size = 10
minibatch_size = 8

save_path = "../trandata/ppo_model.pth"

def save_model(model, optimizer, epoch, save_path):
    torch.save({
        'epoch': epoch,
        'model_state_dict': model.state_dict(),
        'optimizer_state_dict': optimizer.state_dict()
    }, save_path)

def load_model(model, optimizer, save_path):
    if os.path.exists(save_path):
        checkpoint = torch.load(save_path)
        model.load_state_dict(checkpoint['model_state_dict'])
        optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
        start_epoch = checkpoint['epoch']
        print(f"Model loaded from {save_path}, starting from epoch {start_epoch}")
        return start_epoch
    else:
        print(f"No model found at {save_path}, starting from scratch")
        return 0
    
# the model is append
def append_reward_to_csv(file_path, rewards, throughput, latancy, bler, prbs, each_time, totel_times):
    file_exists = os.path.isfile(file_path)
    with open(file_path, mode='a', newline='') as file:
        writer = csv.writer(file)
        #if not file_exists:
        writer.writerow(['Reward', 'Throughput', 'Latancy', 'BLER', 'PRBs', 'Each_time', 'Totel_time'])
        #writer.writerow(['end', 'end', 'end', 'end', 'end', 'end'])
        for reward, throughput, latancy, bler, prbs, each_time, totel_times in zip(rewards, throughput, latancy, bler, prbs, each_time, totel_times):
            writer.writerow([reward, throughput, latancy, bler, prbs, each_time, totel_times])
    print(f"data saved to  {file_path}")

def append_score_to_csv(file_path, scores, optimization_steps, num_state, times):
    file_exists = os.path.isfile(file_path)
    with open(file_path, mode='a', newline='') as file:
        writer = csv.writer(file)        
        if not file_exists:
            writer.writerow(['Score', 'Optimization_steps', 'Num_state', 'Time'])
        writer.writerow(['end', 'end', 'end', 'end'])       
        for scores, optimization_steps, num_state, times in zip(scores, optimization_steps, num_state, times):
            writer.writerow([scores, optimization_steps, num_state, times])
    print(f"data saved to  {file_path}")

def signal_handler(sig, frame):
    print("system shut down, data saved to ...")
    append_reward_to_csv('../trandata/rewards.csv', rewards, throughput, latancy, bler, prbs, each_time,  totel_times)
    #append_score_to_csv('../trandata/scores.csv', scores, optimization_steps, num_state, times)
    #save_data(rewards, '../trandata/rewards.csv')
    #save_data(scores, '../trandata/scores.csv')
    sys.exit(0)

plt.ion()  
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(5, 4))
rewards = []
throughput = []
latancy = []
bler = []
prbs = []
each_time = []
totel_times = []

scores = []
optimization_steps = []
num_state = []
times = []

class PPO(nn.Module):
    def __init__(self):
        super(PPO, self).__init__()
        self.data = []

        embadding_input = 6
        hidden_layer = 8
        slice_num = 3

        self.fc1 = nn.Linear(embadding_input, hidden_layer)
        self.fc_mu = nn.Linear(hidden_layer, slice_num)
        self.fc_std = nn.Linear(hidden_layer, slice_num)
        self.fc_v = nn.Linear(hidden_layer, 1)
        self.optimizer = optim.Adam(self.parameters(), lr=learning_rate)
        self.optimization_step = 0

    def pi(self, x, softmax_dim=0):
        x = F.relu(self.fc1(x))
        mu = torch.sigmoid(self.fc_mu(x))
        std = F.softplus(self.fc_std(x))
        return mu, std

    def v(self, x):
        x = F.relu(self.fc1(x))
        v = self.fc_v(x)
        return v

    def put_data(self, transition):
        self.data.append(transition)

    def make_batch(self):
        s_batch, a_batch, r_batch, s_prime_batch, prob_a_batch, done_batch = [], [], [], [], [], []
        data = []

        for j in range(buffer_size):
            for i in range(minibatch_size):
                rollout = self.data.pop()
                s_lst, a_lst, r_lst, s_prime_lst, prob_a_lst, done_lst = [], [], [], [], [], []

                for transition in rollout:
                    s, a, r, s_prime, prob_a, done = transition

                    s_lst.append(s)
                    a_lst.append([a])
                    r_lst.append([r])
                    s_prime_lst.append(s_prime)
                    prob_a_lst.append([prob_a])
                    done_mask = 0 if done else 1
                    done_lst.append([done_mask])

                s_batch.append(s_lst)
                a_batch.append(a_lst)
                r_batch.append(r_lst)
                s_prime_batch.append(s_prime_lst)
                prob_a_batch.append(prob_a_lst)
                done_batch.append(done_lst)

            mini_batch = torch.tensor(s_batch, dtype=torch.float), torch.tensor(a_batch, dtype=torch.float), \
                         torch.tensor(r_batch, dtype=torch.float), torch.tensor(s_prime_batch, dtype=torch.float), \
                         torch.tensor(done_batch, dtype=torch.float), torch.tensor(prob_a_batch, dtype=torch.float)
            data.append(mini_batch)

        return data

    def calc_advantage(self, data):
        data_with_adv = []
        for mini_batch in data:
            s, a, r, s_prime, done_mask, old_log_prob = mini_batch
            with torch.no_grad():
                td_target = r + gamma * self.v(s_prime) * done_mask
                delta = td_target - self.v(s)
            delta = delta.numpy()

            advantage_lst = []
            advantage = 0.0
            for delta_t in delta[::-1]:
                advantage = gamma * lmbda * advantage + delta_t[0]
                advantage_lst.append([advantage])
            advantage_lst.reverse()
            advantage = torch.tensor(advantage_lst, dtype=torch.float)
            data_with_adv.append((s, a, r, s_prime, done_mask, old_log_prob, td_target, advantage))

        return data_with_adv

    def train_net(self):
        if len(self.data) == minibatch_size * buffer_size:
            data = self.make_batch()
            data = self.calc_advantage(data)

            for i in range(K_epoch):
                for mini_batch in data:
                    s, a, r, s_prime, done_mask, old_log_prob, td_target, advantage = mini_batch

                    mu, std = self.pi(s, softmax_dim=1)
                    dist = Normal(mu, std)
                    a_squeezed = a.squeeze(dim=2)
                    log_prob = dist.log_prob(a_squeezed)#.mean(dim = 2, keepdim=True)
                    #log_prob = log_prob_dim.squeeze(dim = 2)
                    old_log_prob_dim = old_log_prob.squeeze()
                    ratio = torch.exp(log_prob - old_log_prob_dim)  # a/b == exp(log(a)-log(b))
                    #print("ratio.size()", ratio.size())
                    
                    surr1 = ratio * advantage
                    surr2 = torch.clamp(ratio, 1 - eps_clip, 1 + eps_clip) * advantage
                    #loss = -torch.min(surr1, surr2) + F.smooth_l1_loss(self.v(s), td_target)

                    loss = -torch.min(surr1, surr2).mean() + F.smooth_l1_loss(self.v(s), td_target)

                    surr1_first_action = surr1[:, :, 0]
                    surr1_second_action = surr1[:, :, 1]

                    surr2_first_action = surr2[:, :, 0]
                    surr2_second_action = surr2[:, :, 1]

                    #print( "-torch.min(surr1_first_action, surr2_first_action)", -torch.min(surr1_first_action, surr2_first_action))

                    #loss = -0.9*torch.min(surr1_first_action, surr2_first_action) - 0.1*torch.min(surr1_second_action, surr2_second_action) + F.smooth_l1_loss(self.v(s), td_target)
                    #-torch.min(surr1_first_action, surr2_first_action)
                    #- torch.min(surr1_second_action, surr2_second_action)
                    
                    self.optimizer.zero_grad()
                    loss.mean().backward()
                    nn.utils.clip_grad_norm_(self.parameters(), 1.0)
                    self.optimizer.step()
                    self.optimization_step += 1

def add_ue_to_slice(slice_data, ue_data, num):
    slice_data[num].append(ue_data)

def main():
    env = slice_env()
    env.initlize(user_num)
    model = PPO()
    optimizer = model.optimizer
    start_epoch = 0
    #start_epoch = load_model(model, optimizer, save_path)
    

    #gnn = DynamicGNN(in_channels=10, hidden_channels=16, out_channels=5)
    gnn = DynamicGNN(in_channels=10, out_channels=2)
    gnn_optimizer = torch.optim.Adam(gnn.parameters(), lr=0.01)

    
    score = 0.0
    print_interval = 1
    rollout = []
    env.init_file()
    numbersofstate = 0

    #三个slice分别对应 eMBB, URLLC, mMTC

    slice_data_list = [
        [
            0,
            [
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            ]
        ],
        [                         
            0,
            [
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            ]
        ],
        [
            0,
            [
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            ]
        ]
    ]
    
    signal.signal(signal.SIGINT, signal_handler)

    #initial
    s_prime = np.array([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1])
    s = np.array([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1])
    #epoch_times = []
    total_start_time = time.time()

    #try:
    for n_epi in range(start_epoch, 10000):
        
        #epoch_start_time = time.time()

        done = False
        count = 0
        while count < 50 and not done:
            roll_time_begain = time.time()
            for t in range(rollout_len):
                mu, std = model.pi(torch.from_numpy(s).float())
                dist = Normal(mu, std)
                a = dist.sample()
                log_prob = dist.log_prob(a)
                '''
                esti_slice1 = int(a[0].item() * 100)
                esti_slice2 = int(a[1].item() * 100)
                esti_slice3 = int(a[2].item() * 100)
                print(esti_slice1, esti_slice2, esti_slice3)
                #s = np.array([0.9, 0.1, 0.9, 0.1, 0.9, 0.1])
                if esti_slice1 < 20 or esti_slice1 > 80 or esti_slice2 < 20 or esti_slice2 > 80 or esti_slice3 < 20 or esti_slice3 > 80:
                    slice1 = 20
                    slice2 = 20
                    slice3 = 20
                else:
                    slice1 = esti_slice1
                    slice2 = esti_slice2
                    slice3 = esti_slice3
                '''
                slice = [0] * len(a)
                action = a
                
                for i in range(0, 3):
                    if a[i] < 0:
                        a[i] = 0.1
                    totle = sum(a[:3]).item()
                    slice[i] = int( a[i].item() / totle * 100)
                    slice_data_list[i][0] = slice[i]

                    if slice[i] < 10:
                        slice[i] = 10
                    if slice[i] > 90:
                        slice[i] = 90 

                env.send_action(slice[0], slice[1], slice[2], 0)  #for real system
                time.sleep(0.1)
                ue_mac  = env.get_all_state(user_num)
                numbersofstate = numbersofstate + 1
                for i in range(0, user_num):
                    #print(ue_mac[i])
                    slice_data_list[i % 3][1][int(i/3)] = ue_mac[i]

                done = 0
                t, l, b, p, r, done = env.caculate_reward(a, user_num)
                #t, l, b,  p, r, done = env.caculate_regret(a, user_num)
                #p, tao, bl, regret, done = env.caculate_regret(a, user_num)
                #r = regret

                print(slice[0], slice[1], slice[2], r)
                #print("action", a)

                for i in range(0, 2):
                    if slice[i] < 10:
                        r = 0
                    if slice[i] > 90:
                        r = 0

                gcn_loss = (100-r) * 0.1
                out = gnn.update_graph(slice_data_list, gnn, gnn_optimizer, gcn_loss)
                s = out.view(-1).detach().numpy()

                rollout.append((s, a.tolist(), r, s_prime, log_prob.tolist(), done))
                s_prime = s

                if len(rollout) == rollout_len:
                    model.put_data(rollout)
                    rollout = []

                #s = s_prime
                #score += r
                count += 1
                rewards.append(r)
                throughput.append(t)
                latancy.append(l)
                bler.append(b)
                prbs.append(p)
                roll_time_end = time.time()
                each_time.append(roll_time_end - roll_time_begain)
                totel_times.append(roll_time_end - total_start_time)
   
            model.train_net()

        ax1.clear()
        print(r)
        ax1.plot(throughput, label='throughput')
        ax1.legend()
        plt.pause(0.001)
        '''
        epoch_end_time = time.time()
        epoch_duration = epoch_start_time - epoch_end_time
        epoch_times.append(epoch_duration)

        total_elapsed_time = epoch_end_time - total_start_time

        
        if n_epi % print_interval == 0 and n_epi != 0:
            print("# of episode :{}, avg score : {:.1f}, optmization step: {} took {:.2f} seconds".format(n_epi, score / print_interval,
                                                                                      model.optimization_step, epoch_duration))
            print("numbersofstate", numbersofstate)
            print(f"Total time elapsed until epoch {n_epi + 1}: {total_elapsed_time:.2f} seconds")
            
            scores.append(score)
            optimization_steps.append(model.optimization_step)
            num_state.append(numbersofstate)
            times.append(total_elapsed_time)

            ax2.clear()
            ax2.plot(scores, label='Score')
            ax2.legend()
            plt.pause(0.001)
                
            score = 0.0
            save_model(model, optimizer, n_epi, save_path)
        '''

    env.close()
    plt.ioff()
    plt.show()

'''
    except IndexError as e:
        print(f"IndexError: {e}")
        save_data(rewards, '../trandata/rewards.csv')
        save_data(scores, '../trandata/scores.csv')
        sys.exit(1)
        
    except Exception as e:
        print(f"Exception: {e}")
        save_data(rewards, '../trandata/rewards.csv')
        save_data(scores, '../trandata/scores.csv')
        sys.exit(1)
'''
if __name__ == '__main__':
    main()
