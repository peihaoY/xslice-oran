# Re-attempting to execute the provided code after confirming the installation of torch_geometric

# Required libraries and dependencies
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch_geometric.data import Data
from torch_geometric.nn import GCNConv, global_mean_pool
from torch.utils.data.sampler import BatchSampler, SubsetRandomSampler
import random
from env import slice_env 

# Define the DynamicGNN class
class DynamicGNN(nn.Module):
    def __init__(self, in_channels, out_channels):
        super(DynamicGNN, self).__init__()
        self.conv1 = GCNConv(in_channels, 32)
        self.conv2 = GCNConv(32, 16)
        self.conv3 = GCNConv(16, 16)
        self.conv4 = GCNConv(16, out_channels)

    def forward(self, data):
        x, edge_index, batch = data.x, data.edge_index, data.batch
        #print(edge_index, batch)
        x = self.conv1(x, edge_index)
        x = F.relu(x)
        x = self.conv2(x, edge_index)
        x = F.relu(x)
        x = self.conv3(x, edge_index)
        x = F.relu(x)
        x = self.conv4(x, edge_index)
        x = global_mean_pool(x, batch)
        return x
    
    # Define the create_graph function
    def create_graph(self, slice_data_list):
        node_features = []
        edge_indices = []
        batch = []
        current_batch = 0

        for slice_data in slice_data_list:
            num_nodes = len(slice_data[1])
            for user in slice_data[1]:
                features = user
                node_features.append(features)

            edge_index = torch.tensor([[], []], dtype=torch.long)
            for i in range(num_nodes):
                for j in range(num_nodes):
                    if i != j:
                        edge_index = torch.cat(
                            [edge_index, torch.tensor([[i + current_batch], [j + current_batch]], dtype=torch.long)], dim=1)
            edge_indices.append(edge_index)
    
            batch += [current_batch] * num_nodes
            current_batch += 1 #num_nodes  

        x = torch.tensor(node_features, dtype=torch.float)
        edge_index = torch.cat(edge_indices, dim=1)
        batch = torch.tensor(batch, dtype=torch.long)
        #print(batch)
        data = Data(x=x, edge_index=edge_index, batch=batch)
        #print(data.size())
        return data
    '''
    # Define the update_user_features function
    def update_user_features(self, slice_data_list):
        for slice_data in slice_data_list:
            for user in slice_data['users']:
                user['CQI'] = max(0, min(15, user['CQI'] + random.randint(-1, 1)))
                user['MCS'] = max(0, min(28, user['MCS'] + random.randint(-1, 1)))
                user['BER'] = max(0.0, user['BER'] + random.uniform(-0.005, 0.005))
                user['SNR'] = max(0, user['SNR'] + random.uniform(-1, 1))
                user['throughput'] = max(0, user['throughput'] + random.uniform(-5, 5))

    # Sample slice data initialization
    slice_data_list = [
        {
        'resource_blocks': 10,
        'users': [
            {'CQI': 10, 'MCS': 4, 'BER': 0.01, 'SNR': 20, 'throughput': 50},
            {'CQI': 12, 'MCS': 5, 'BER': 0.02, 'SNR': 22, 'throughput': 55},
            {'CQI': 15, 'MCS': 6, 'BER': 0.015, 'SNR': 25, 'throughput': 60}
            ]
        },
        {
        'resource_blocks': 8,
        'users': [
            {'CQI': 9, 'MCS': 3, 'BER': 0.02, 'SNR': 18, 'throughput': 45},
            {'CQI': 11, 'MCS': 4, 'BER': 0.025, 'SNR': 21, 'throughput': 50}
            ]
        },
        {
        'resource_blocks': 12,
        'users': [
            {'CQI': 14, 'MCS': 5, 'BER': 0.01, 'SNR': 23, 'throughput': 55},
            {'CQI': 13, 'MCS': 4, 'BER': 0.03, 'SNR': 22, 'throughput': 52},
            {'CQI': 16, 'MCS': 6, 'BER': 0.02, 'SNR': 26, 'throughput': 65},
            {'CQI': 10, 'MCS': 3, 'BER': 0.015, 'SNR': 20, 'throughput': 48}
            ]
        }
    ]
    '''
    # Define a function to update the graph
    def update_graph(self, slice_data_list, gnn, optimizer, gcn_loss):
        gnn.train()
        data = self.create_graph(slice_data_list)
        optimizer.zero_grad()
        out = gnn(data)
        #loss = out.mean()  # Placeholder for actual loss calculation
        loss = F.mse_loss(out, torch.zeros_like(out)) #peihao I have to change the loss function
        loss.backward()
        optimizer.step()
        return out

'''
# Instantiate the GNN and optimizer
gnn = DynamicGNN(in_channels=5, hidden_channels=16, out_channels=2)
gnn_optimizer = torch.optim.Adam(gnn.parameters(), lr=0.01)

# Real-time GNN update loop
for time_step in range(100):  # Assume 100 time steps

    #update_user_features(slice_data_list)

    if time_step % 10 == 0:
       new_user = {'CQI': 8 + time_step % 3, 'MCS': 3 + time_step % 2, 'BER': 0.02 - 0.001 * (time_step % 5),
                'SNR': 18 + time_step % 4, 'throughput': 45 + 5 * (time_step % 3)}
       slice_data_list[time_step % 3]['users'].append(new_user)

    out = update_graph(slice_data_list, gnn, gnn_optimizer)
    #print(len(out))
    print(f'Time step {time_step}, Slice embedding: {out}')
'''
