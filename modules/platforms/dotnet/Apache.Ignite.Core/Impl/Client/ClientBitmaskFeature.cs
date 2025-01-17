/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace Apache.Ignite.Core.Impl.Client
{
    /// <summary>
    /// Client feature ids. Values represent the index in the bit array.
    /// Unsupported flags must be commented out.
    /// </summary>
    internal enum ClientBitmaskFeature
    {
        // UserAttributes = 0,
        ExecuteTaskByName = 1,
        // ClusterStates = 2,
        ClusterGroupGetNodesEndpoints = 3,
        ClusterGroups = 4,
        ServiceInvoke = 5, // The flag is not necessary and exists for legacy reasons
        // DefaultQueryTimeout = 6, // IGNITE-13692
        QueryPartitionsBatchSize = 7,
        BinaryConfiguration = 8,
        ServiceInvokeCtx = 10
    }
}
